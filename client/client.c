#include "include/client.h"
#include "include/config.h"
#include "include/directory.h"
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

void run_client()
{
    log_i("%s", "Starting client...");

    //обработчик системного вызова
    signal(SIGINT, handle_signal);

    //дирректория с письмами
    maildir_main *maildir;
    context client_context = {0};

    //инициализируем дирректорию с письмами
    maildir = init_maildir(config_context.maildir);
    log_i("maildir %s", maildir->directory);
    //sleep(1);
    //int iter = 1;
    while (1)
    {
        printf("Start iter\n");
        sleep(10);
        int newServers = read_maildir_servers(maildir);
        log_i("Find new servers: %d ", newServers);

        if (newServers > 0)
        {
            for (int j = 0; j < maildir->servers_size; j++)
            {
                if (maildir->servers[j].messages_count > 0)
                {
                    if (maildir->servers[j].is_connected != 1)
                    {
                        sleep(1);
                        //выполняем подключение к серверу
                        log_d("Try connect to server %s", maildir->servers[j].server_name);
                        maildir->servers[j].smtp_context = smtp_connect(maildir->servers[j].server_name, "25");

                        if (maildir->servers[j].smtp_context->state_code == INVALID)
                        {
                            log_e("Error connect to %s", maildir->servers[j].server_name);
                            free(maildir->servers[j].smtp_context);
                            break;
                        }
                        //cont->state_code = CONNECT
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
                    }
                }
            }
        }

        printf("Do poll\n");
        int res = poll(client_context.poll_fds, client_context.conn_context_count, 1000);
        if (res == 1)
        {
            log_e("%s", "Error when poll");
        }
        else if (res == 0)
        {
            log_i("%s", "timeout");
        }
        else
        {
            log_d("ready from poll = %d", res);
            for (int i = 0; i < client_context.conn_context_count; i++)
            {
                if (client_context.poll_fds[i].revents != 0)
                {
                    // prepare msg
                }
            }
        }

        //  sleep(10);
        //if (iter == 2)
        //{
            break;
        // }
        // iter++;
    }
}

void handle_signal(int signal)
{
    printf("%s\n", "Client process stopped");
    //close all sockets
    exit(0);
}

