#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include "include/config.h"
#include "include/util.h"
#include "include/smtp.h"
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <log.h>
#include <resolv.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <libgen.h>
#include <unistd.h>

smtp_context *smtp_connect(char *server, char *port)
{

    smtp_context *context;

    context = malloc(sizeof(smtp_context));
    context->socket_desc = -1;
    context->state_code = INVALID;

    ips ips = {0};
    ips.ip_array = NULL;

    char *mxs[MAX_MX_ADDRS];

    int len = resolve_mx(server, mxs, MAX_MX_ADDRS);

    if (len == -1)
    {
        log_e("MX-record for domain %s Not found", server);
        return context;
    }

    for (int i = 0; i < len; i++)
    {
        ips = get_ips_by_hostname(mxs[i]);

        int server_socket;
        if ((server_socket = connect_to_server(ips, port)) != -1)
        {
            context->socket_desc = server_socket;
            context->state_code = CONNECT;

            struct timespec tv = {0};
            tv.tv_sec = 1;
            nanosleep(&tv, &tv);

            char *addr = get_addr_by_socket(server_socket);
            log_i("Success connect to %s by address: %s", server, addr);
            free(addr);
            break;
        }
    }

    for (int l = 0; l < len; l++)
    {
        free(mxs[l]);
    }

    for (int l = 0; l < ips.ips_size; l++)
    {
        free(ips.ip_array[l]);
    }

    free(ips.ip_array);
    return context;
}

int connect_to_server(ips ips, char *port)
{
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (server_socket == -1)
    {
        log_e("%s", "Memmory don`t allocate for socket");
        return -1;
    }

    /* Structure describing an Internet socket address.  */
    struct sockaddr_in server_addr;

    server_addr.sin_family = PF_INET; //  IP protocol family. 
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    server_addr.sin_port = htons(convert_string_to_long_int(port));

    for (int j = 0; j < ips.ips_size; j++)
    {

        //преобразовывает обычный вид IP-адреса (из номеров и точек) в двоичный код
        inet_aton(ips.ip_array[j], &(server_addr.sin_addr));

        socklen_t socklen = sizeof(struct sockaddr);

        if (connect(server_socket, (struct sockaddr *)&server_addr, socklen) == -1)
        {
            log_e("Error open connection with SMTP-server by address %d ", server_addr.sin_addr.s_addr);
            continue;
        }
        else
        {
            return server_socket;
        }
    }

    return -1;
}

ips get_ips_by_hostname(char *hostname)
{
    struct hostent *hostent;
    struct in_addr **addr_list;
    ips ips = {0};

    if ((hostent = gethostbyname(hostname)) == NULL)
    {
        //log_e("%s", "Error find address by domain name");
        return ips;
    }

    addr_list = (struct in_addr **)hostent->h_addr_list;

    ips.ip_array = allocate_memory(sizeof(char *));
    for (int i = 0; addr_list[i] != NULL; i++)
    {
        ips.ip_array = reallocate_memory(ips.ip_array, sizeof(char *) * ips.ips_size,
                                         sizeof(char *) * (ips.ips_size + 1));
        ips.ip_array[i] = strdup(inet_ntoa(*addr_list[i]));
        ips.ips_size++;
    }

    return ips;
}

int resolve_mx(const char *name, char **mxs, int limit)
{
    //log_i("resolve MX record for %s", name);
    u_char response[NS_PACKETSZ];
    ns_msg msg;
    ns_rr rr;
    int mx_index, ns_index, len;
    char dispbuf[4096];

    //делаем запрос к днс. по имени получаем полное доменное имя
    if ((len = res_search(name, C_IN, T_MX, response, NS_PACKETSZ + 1)) < 0)
    {
        log_e("Error find MX record for %s", name);
        return -1;
    }

    //заполняем msg
    if (ns_initparse(response, len, &msg) < 0)
    {
        log_e("Error find MX record for %s", response);
        return 0;
    }


    len = ns_msg_count(msg, ns_s_an);
    if (len < 0)
        return 0;

    for (mx_index = 0, ns_index = 0; mx_index < limit && ns_index < len; ns_index++)
    {
        if (ns_parserr(&msg, ns_s_an, ns_index, &rr)) //извлекаем полученную информацию
        {
            log_e("%s", "resolve_mx() ns_parserr()");
            continue;
        }

        ns_sprintrr(&msg, &rr, NULL, NULL, dispbuf, sizeof(dispbuf));
        if (ns_rr_class(rr) == ns_c_in && ns_rr_type(rr) == ns_t_mx)
        {
            char mxname[MAXDNAME];
            // расширяет сжатое доменное имя до полного доменного имени
            dn_expand(ns_msg_base(msg), ns_msg_base(msg) + ns_msg_size(msg), ns_rr_rdata(rr) + NS_INT16SZ,
                      mxname, sizeof(mxname));
            mxs[mx_index++] = strdup(mxname);
        }
    }

    return mx_index;
}

char *get_addr_by_socket(int socket)
{
    struct sockaddr_in client_addr = {0};
    socklen_t s_len = sizeof(struct sockaddr);

    if (getpeername(socket, (struct sockaddr *)&client_addr, &s_len) == -1)
    {
        log_e("%s", "Unable to find name by socket");
        return NULL;
    }

    char *addr;
    asprintf(&addr, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    return addr;
}

smtp_response get_smtp_response(smtp_context *context)
{
    smtp_response response;
    char *buffer = NULL;
    response.message = NULL;

    char *addr = get_addr_by_socket(context->socket_desc);

    if ((buffer = read_from_socket(context->socket_desc)) == NULL)
    {
        //log_e("Error while read buffer for (%s)", addr);
        response.status_code = UNDEFINED_ERROR;
        free(addr);
        return response;
    }

    char code[3];
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        if (buffer[i] == ' ')
        {
            break;
        }
        code[i] = buffer[i];
    }

    char *message = allocate_memory(strlen(buffer) - i + 1);
    strncpy(message, buffer + i + 1, strlen(buffer) - i);
    message[strlen(buffer) - i] = 0;

    response.message = message;
    response.status_code = convert_string_to_long_int(code);

    //log_i("Response <%s>: %d %s", addr, response.status_code, response.message);
    printf("Response <%s>: %d %s \n", addr, response.status_code, response.message);

    free(addr);
    free(buffer);
    return response;
}

char *read_from_socket(int socket_d)
{
    int end_sim_count = 0;

    size_t buff_size = 4;

    char *buff = allocate_memory(sizeof(char) * buff_size);
    memset(buff, 0, buff_size);
    char *ptr = buff;
    int recv_size = 0;

    int bytes;
    while ((bytes = recv(socket_d, ptr, 1, 0)) > 0)
    {
        recv_size++;

        if (*ptr == '\0')
        {
            free(buff);
            return NULL;
        }

        if (*ptr == '\n' || *ptr == '\r')
        {
            end_sim_count++;
        }
        ptr++;

        if (end_sim_count == 2)
        {
            *(ptr - 2) = '\0';
            return buff;
        }

        if (recv_size == buff_size - 2)
        {
            size_t prev_size = buff_size;
            buff_size += (buff_size / 2);
            buff = reallocate_memory(buff, sizeof(char) * prev_size, sizeof(char) * buff_size);
            ptr = buff + recv_size;
        }
    }

    if (bytes <= 0)
    {
        free(buff);
        return NULL;
    }

    return buff;
}

int write_to_socket(int socket_d, char *message)
{
    char *ptr = message;
    size_t msg_len = strlen(message);
    while (msg_len > 0)
    {
        int send_size = send(socket_d, message, msg_len, MSG_NOSIGNAL);
        if (send_size == -1)
        {
            return 0;
        }
        msg_len -= send_size;
        ptr += send_size;
    }
    return 1;
}

state_code send_smtp_command(smtp_context *smtp_cont, char *str)
{
    if (!write_to_socket(smtp_cont->socket_desc, str))
    {
        smtp_cont->state_code = INVALID;
    }

    return smtp_cont->state_code;
}

state_code smtp_send_helo(smtp_context *context)
{
    char *buff;
    asprintf(&buff, "HELO %s\r\n", config_context.hostname);

    if (send_smtp_command(context, buff) != INVALID)
    {
        context->state_code = HELO;

        buff[strlen(buff) - 1] = 0;

        char *addr = get_addr_by_socket(context->socket_desc);

        //sleep(1);
        //log_i("Command <%s>: %s", addr, buff);
        printf("Command <%s>: %s \n", addr, buff);
        free(addr);
    }
    free(buff);
    return context->state_code;
}

state_code smtp_send_mail(smtp_context *context, char *from)
{
    char *buff;
    asprintf(&buff, "MAIL from:<%s>\r\n", from);

    if (send_smtp_command(context, buff) != INVALID)
    {
        context->state_code = MAIL;

        buff[strlen(buff) - 1] = 0;

        char *addr = get_addr_by_socket(context->socket_desc);

        //sleep(1);
        //log_i("Command <%s>: %s ", addr, buff);
        printf("Command <%s>: %s \n", addr, buff);
        free(addr);
    }

    free(buff);
    return context->state_code;
}

state_code smtp_send_rcpt(smtp_context *context, char *to)
{
    char *buff;
    asprintf(&buff, "RCPT to:<%s>\r\n", to);

    if (send_smtp_command(context, buff) != INVALID)
    {
        context->state_code = RCPT;

        buff[strlen(buff) - 1] = 0;

        char *addr = get_addr_by_socket(context->socket_desc);

        //sleep(1);
        //log_i("Command <%s>: %s ", addr, buff);
        printf("Command <%s>: %s \n", addr, buff);

        free(addr);
    }

    free(buff);
    return context->state_code;
}

state_code smtp_send_data(smtp_context *context)
{
    if (send_smtp_command(context, "DATA\r\n") != INVALID)
    {
        context->state_code = DATA;

        char *addr = get_addr_by_socket(context->socket_desc);

        //sleep(1);
        //log_i("Command <%s>: %s ", addr, "DATA");
        printf("Command <%s>: %s \n", addr, "DATA");
        free(addr);
    }

    return context->state_code;
}

state_code smtp_send_message(smtp_context *context, char *message)
{
    if (send_smtp_command(context, message) != INVALID)
    {
        context->state_code = MESSAGE;

        char *buff;
        asprintf(&buff, "%s", message);

        char *addr = get_addr_by_socket(context->socket_desc);

        //sleep(1);
        //log_i("Command <%s>: %s ", addr, buff);
        printf("Command <%s>: %s \n", addr, buff);

        free(addr);
        free(buff);
    }

    return context->state_code;
}

state_code smtp_send_end_message(smtp_context *context)
{
    if (send_smtp_command(context, "\r\n.\r\n") != INVALID)
    {
        context->state_code = END_MESSAGE;

        char *addr = get_addr_by_socket(context->socket_desc);

        //sleep(1);
        //log_i("Command <%s>: %s ", addr, ".");
        printf("Command <%s>: %s \n", addr, ".");

        free(addr);
    }

    return context->state_code;
}

state_code smtp_send_quit(smtp_context *context)
{
    if (send_smtp_command(context, "QUIT\r\n") != INVALID)
    {
        context->state_code = QUIT;

        char *addr = get_addr_by_socket(context->socket_desc);

        //log_i("Command <%s>: %s ", addr, "QUIT");
        printf("Command <%s>: %s \n", addr, "QUIT");
        free(addr);
    }

    return context->state_code;
}

int is_smtp_success(status_code status_code)
{
    if (status_code < 400)
    {
        return 1;
    }
    return 0;
}

int is_smtp_4yz_code(status_code status_code)
{
    if (status_code > 399 && status_code < 500)
    {
        return 1;
    }
    return 0;
}

int is_smtp_5yz_code(status_code status_code)
{
    if (status_code > 499)
    {
        return 1;
    }
    return 0;
}
