#include "include/client.h"
#include "include/config.h"
#include "include/util.h"
#include "include/smtp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <libgen.h>
#include <unistd.h>
#include <log.h>
#include <poll.h>

context client_context = {0};
void run_client()
{
    log_i("%s", "Starting client...");

    //регистрируем обработчик системного вызова
    signal(SIGINT, handle_signal);

    //дирректория с письмами
    maildir_main *maildir;

    //инициализируем дирректорию с письмами
    maildir = init_maildir(config_context.maildir);
    client_context.maildir = maildir;
    log_i("maildir %s", maildir->directory);
    while (1)
    {
        int newServers = read_maildir_servers(maildir);
        log_i("Find new servers: %d ", newServers);

        for (int j = 0; j < maildir->servers_size; j++)
        {
            if (maildir->servers[j].messages_count > 0)
            {
                if (maildir->servers[j].is_connected != 1)
                {
                    //выполняем подключение к серверу
                    log_i("Try connect to server %s", maildir->servers[j].server_name);
                    printf("Try connect to server %s\n", maildir->servers[j].server_name);

                    maildir->servers[j].smtp_context = smtp_connect(maildir->servers[j].server_name, "25");

                    if (maildir->servers[j].smtp_context->state_code == INVALID)
                    {
                        log_e("Error connect to %s", maildir->servers[j].server_name);
                        free(maildir->servers[j].smtp_context);
                        break;
                    }

                    maildir->servers[j].is_connected = 1;
                    client_context.conn_context_count++;

                    if (client_context.conn_context_count == 1)
                    {
                        client_context.poll_fds = malloc(sizeof(struct pollfd));
                    }
                    else
                    {
                        client_context.poll_fds = reallocate_memory(client_context.poll_fds,
                                                                    sizeof(struct pollfd) * (client_context.conn_context_count - 1),
                                                                    sizeof(struct pollfd) * client_context.conn_context_count);
                    }

                    struct pollfd poll_fd = {0};
                    poll_fd.fd = maildir->servers[j].smtp_context->socket_desc;
                    poll_fd.events = POLL_IN;

                    client_context.poll_fds[client_context.conn_context_count - 1] = poll_fd;
                    client_context.map[client_context.conn_context_count - 1] = j;
                }
            }
            // log_i("Searching new mail for %s ... \n", maildir->servers[j].server_name);
            // printf("Searching new mail for %s ... \n", maildir->servers[j].server_name);
            read_maildir_servers_new(&maildir->servers[j]);
        }

        if (client_context.conn_context_count > 0)
        {
            int res = poll(client_context.poll_fds, client_context.conn_context_count, 100);
            if (res == -1)
            {
                log_e("%s", "Error when poll");
                printf("Error when poll");
            }
            else if (res == 0)
            {
                log_i("%s", "timeout");
            }
            else
            {
                for (int i = 0; i < client_context.conn_context_count; i++)
                {
                    int index = client_context.map[i];
                    //подготавливаем сообщение для отправки
                    if (client_context.maildir->servers[index].cur_msg == NULL)
                    {
                        message *msg;
                        if ((msg = get_message(&client_context.maildir->servers[index])) == NULL)
                        {

                            log_i("msg not found in %s", client_context.maildir->servers[index].server_name);
                            smtp_send_quit(client_context.maildir->servers[index].smtp_context);
                            remove_server_from_context(i);

                            continue;
                        }
                        else
                        {
                            client_context.maildir->servers[index].cur_msg = msg;
                        }
                    }

                    if (client_context.poll_fds[i].revents != 0)
                    {
                        client_context.poll_fds[i].revents = 0;
                        switch (client_context.maildir->servers[index].smtp_context->state_code)
                        {
                        case CONNECT:
                        {
                            handle_connect(index);
                            break;
                        }
                        case HELO:
                        {
                            handler_helo(index);
                            break;
                        }
                        case MAIL:
                        {
                            handler_mail(index);
                            break;
                        }
                        case RCPT:
                        {
                            handler_rcpt(index);
                            break;
                        }
                        case DATA:
                        {
                            handler_data(index);
                            break;
                        }
                        case END_MESSAGE:
                        {
                            handler_end_message(index);
                            break;
                        }
                        default:
                        {
                            printf("state_code = %d\n", client_context.maildir->servers[index].smtp_context->state_code);
                            remove_server_from_context(index);
                            break;
                        }
                        }
                    }
                    else
                    {
                        log_i("some error with  %s", client_context.maildir->servers[index].server_name);
                    }
                }
            }
        }
    }
}

void handle_signal(int signal)
{

    log_i("%s\n", "Client process stopping...");
    printf("Client process stopping...\n");

    for (int i = 0; i < client_context.conn_context_count; i++)
    {
        maildir_other_server *server = &client_context.maildir->servers[client_context.map[i]];
        if (server->step == 1) //отправили, но не считали ответ
        {

            for (int i = 0; i < server->messages_count; i++)
            {
                free(server->message_full_file_names[i]);
            }
            server->messages_count = 0;

            handler_end_message(client_context.map[i]); // считаем ответ от сервера и закром сессию
        }
        else if (server->step == 2) //отправили, считали ответ, но не удалили сообщение
        {
            if (is_smtp_success(server->smtp_context->state_code))
            {
                delete_msg(server, server->cur_msg, 0);
                server->cur_msg = NULL;
                server->response->status_code = NOT_ANSWER;
                server->response->message = NULL;
                smtp_send_quit(server->smtp_context);
                remove_server_from_context(client_context.map[i]);
            }
            else
            {
                log_i(" STATUS = %d, run %s", server->smtp_context->state_code, "smtp_send_quit(&server->smtp_context);");
                smtp_send_quit(server->smtp_context);
                delete_msg(server, server->cur_msg, 1);
                server->cur_msg = NULL;
                remove_server_from_context(client_context.map[i]);
            }
        }
        else //можем удалять, сообщение еще не отправлено -> в следующий раз отправляем заново
        {
            remove_server_from_context(client_context.map[i]);
        }
    }
    free_context(client_context);

    log_i("%s\n", "Client process stopped");
    printf("Client process stopped");
    exit(0);
}

void free_context()
{
    for (int i = 0; client_context.conn_context_count; i++)
    {
        remove_server_from_context(client_context.map[i]);
    }
    client_context.conn_context_count = 0;

    free_maildir(client_context.maildir);
}

void free_maildir(maildir_main *maildir)
{
    if (maildir != NULL)
    {
        for (int i = 0; i < maildir->servers_size; i++)
        {
            free(maildir->servers[i].server_name);
            free(maildir->servers[i].directory);
            for (int k = 0; k < maildir->servers[i].messages_count; k++)
            {
                free(maildir->servers[i].message_full_file_names[k]);
            }
            free(maildir->servers[i].message_full_file_names);
        }
        maildir->servers_size = 0;
        if (maildir->servers != NULL)
            free(maildir->servers);
        if (maildir->directory != NULL)
            free(maildir->directory);
    }
    if (maildir != NULL)
        free(maildir);
}

status_code get_response(maildir_other_server *server)
{
    server->response = get_smtp_response(server->smtp_context);
    return server->response->status_code;
}

void handle_connect(int index)
{
    maildir_other_server *server = &client_context.maildir->servers[index];
    //strat reading, if not copleated when ctrl+c send reset
    status_code status = get_response(server);

    if (status == NOT_ANSWER)
    {
        return;
    }

    if (is_smtp_success(status))
    {
        server->response->status_code = NOT_ANSWER;
        server->response->message = NULL;

        log_i(" STATUS = %d, run %s", status, "smtp_send_helo(&server->smtp_context);");

        //strat reading, if not copleated when ctrl+c send reset
        smtp_send_helo(server->smtp_context);
    }
    else
    {
        log_i(" STATUS = %d, run %s", status, "smtp_send_quit(&server->smtp_context);");

        smtp_send_quit(server->smtp_context);
        remove_server_from_context(index);
    }
}

void handler_helo(int index)
{

    maildir_other_server *server = &client_context.maildir->servers[index];

    status_code status = get_response(server);

    if (status == NOT_ANSWER)
    {
        return;
    }

    if (is_smtp_success(status))
    {

        server->response->status_code = NOT_ANSWER;
        server->response->message = NULL;

        smtp_send_mail(server->smtp_context, server->cur_msg->from);

        log_i("send email %s", server->server_name);
    }
    else
    {
        log_i(" STATUS = %d, run %s", status, "smtp_send_quit(&server->smtp_context);");

        smtp_send_quit(server->smtp_context);
        remove_server_from_context(index);
    }
}

void handler_mail(int index)
{

    maildir_other_server *server = &client_context.maildir->servers[index];

    status_code status = get_response(server);

    if (status == NOT_ANSWER)
    {
        return;
    }

    if (is_smtp_success(status))
    {
        server->error = 0;
        server->response->status_code = NOT_ANSWER;
        server->response->message = NULL;

        smtp_send_rcpt(server->smtp_context, server->cur_msg->to[0]);
        server->iteration = 1;
    }
    else
    {
        log_i(" STATUS = %d, run %s", status, "smtp_send_quit(&server->smtp_context);");

        smtp_send_quit(server->smtp_context);
        remove_server_from_context(index);
    }
}

void handler_rcpt(int index)
{
    maildir_other_server *server = &client_context.maildir->servers[index];

    status_code status = get_response(server);

    if (status == NOT_ANSWER)
    {
        return;
    }

    if (status == SMTP_USER_MAILBOX_WAS_UNAVAILABLE)
    {
        server->error++;
    }
    if ((is_smtp_success(status) || status == SMTP_USER_MAILBOX_WAS_UNAVAILABLE) && server->error < server->cur_msg->to_size)
    {
        server->response->status_code = NOT_ANSWER;
        server->response->message = NULL;
        if (server->iteration < server->cur_msg->to_size)
        {

            smtp_send_rcpt(server->smtp_context, server->cur_msg->to[server->iteration]);
            server->iteration++;
        }
        else
        {
            server->iteration = 0;

            smtp_send_data(server->smtp_context);
        }
    }
    else
    {
        log_i(" STATUS = %d, run %s", status, "smtp_send_quit(&server->smtp_context);");

        smtp_send_quit(server->smtp_context);
        delete_msg(server, server->cur_msg, 1);
        server->cur_msg = NULL;
        remove_server_from_context(index);
    }
}

void handler_data(int index)
{
    maildir_other_server *server = &client_context.maildir->servers[index];
    status_code status = get_response(server);
    if (status == NOT_ANSWER)
    {
        return;
    }

    if (is_smtp_success(status))
    {
        printf("preparing msg \n");
        char *msg = prepare_message(server);
        printf("preparing msg DONE \n");

        server->step=1;
        smtp_send_message(server->smtp_context, msg);
        smtp_send_end_message(server->smtp_context);

    }
    else
    {
        log_i(" STATUS = %d, run %s", status, "smtp_send_quit(&server->smtp_context);");
        printf(" STATUS = %d, run %s", status, "smtp_send_quit(&server->smtp_context);\n");
        smtp_send_quit(server->smtp_context);
        remove_server_from_context(index);
    }
}

void handler_end_message(int index)
{

    maildir_other_server *server = &client_context.maildir->servers[index];

    status_code status = get_response(server);
    server->step = 2;
    if (status == NOT_ANSWER)
    {
        return;
    }

    if (is_smtp_success(status))
    {
        delete_msg(server, server->cur_msg, 0);
        server->cur_msg = NULL;
        if (server->messages_count > 0)
        {

            server->smtp_context->state_code = CONNECT;
            smtp_send_helo(server->smtp_context);
            server->step = 0;
        }
        else
        {
            server->response->status_code = NOT_ANSWER;
            server->response->message = NULL;

            smtp_send_quit(server->smtp_context);
            remove_server_from_context(index);
            server->step = 0;

            return;
        }
        server->cur_msg = NULL;
    }
    else
    {
        log_i(" STATUS = %d, run %s", status, "smtp_send_quit(&server->smtp_context);");

        smtp_send_quit(server->smtp_context);
        delete_msg(server, server->cur_msg, 1);
        server->cur_msg = NULL;
        remove_server_from_context(index);
    }
}

char *prepare_message(maildir_other_server *server)
{
    char *buff;
    int len_msg = strlen(server->cur_msg->line[0]) + 1;

    buff = allocate_memory(sizeof(char) * len_msg);
    strcpy(buff, server->cur_msg->line[0]);
    int new_len = len_msg;
    for (int i = 1; i < server->cur_msg->line_size; i++)
    {
        new_len = new_len + strlen(server->cur_msg->line[i]) + 1;
        if (len_msg < new_len)
        {
            if (len_msg * 2 < new_len)
            {
                buff = reallocate_memory(buff, sizeof(char *) * len_msg,
                                         sizeof(char *) * new_len);
                len_msg = new_len;
            }
            else
            {
                buff = reallocate_memory(buff, sizeof(char *) * len_msg,
                                         sizeof(char *) * len_msg * 2);
                len_msg = len_msg * 2;
            }
        }
        strcat(buff, server->cur_msg->line[i]);
    }

    return buff;
}

void remove_server_from_context(int index)
{
    maildir_main *maildir = client_context.maildir;
    shutdown(maildir->servers[index].smtp_context->socket_desc, SHUT_RDWR);
    close(maildir->servers[index].smtp_context->socket_desc);

    free_server(&maildir->servers[index]);

    for (int j = index; j < maildir->servers_size - 1; j++)
    {
        maildir->servers[j] = maildir->servers[j + 1];
    }

    maildir->servers_size--;

    int i = -1;
    for (int j = 0; j < client_context.conn_context_count; j++)
    {
        if (client_context.map[j] == index)
        {
            i = j;
            break;
        }
    }
    if (i != -1)
    {
        for (int j = i; j < client_context.conn_context_count - 1; j++)
        {
            client_context.poll_fds[j] = client_context.poll_fds[j + 1];
            client_context.map[j] = client_context.map[j + 1];
        }
    }
    client_context.conn_context_count--;
}

void free_server(maildir_other_server *server)
{
    if (server->cur_msg != NULL)
    {
        free_message(server->cur_msg);
    }
    server->cur_msg = NULL;
    for (int i = 0; i < server->messages_count; i++)
    {
        free(server->message_full_file_names[i]);
    }
    server->messages_count = 0;
    if (server->message_full_file_names != NULL)
        free(server->message_full_file_names);
    if (server->directory != NULL)
        free(server->directory);
    if (server->server_name != NULL)
        free(server->server_name);
    server = NULL;
}