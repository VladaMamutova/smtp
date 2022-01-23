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

    log_i("Try connect to  %s", server);
    smtp_context *context;
    sleep(1);

    context = malloc(sizeof(smtp_context));
    context->socket_desc = -1;
    context->state_code = INVALID;

    ips ips = {0};
    ips.ip_array = NULL;

    sleep(1);

    if (strcmp(server, config_context.hostname) == 0)
    { //подключение к локальному серверу

        ips.ip_array = malloc(sizeof(char *));
        ips.ip_array[0] = "127.0.0.1";
        ips.ips_size++;

        int server_socket;
        if ((server_socket = connect_to_server(ips, config_context.server_port)) != -1)
        {
            context->socket_desc = server_socket;
            context->state_code = CONNECT;

            log_i("Succsecc connect to  %s by address: %s:%s", server, ips.ip_array[0], port);
        }
    }
    else // подключение к удаленным серверам
    {
        char *mxs[MAX_MX_ADDRS];

        int len = resolve_mx(server, mxs, MAX_MX_ADDRS);

        if (len == -1)
        {
            log_e("MX-recordfor domain %s Not found", server);
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

    server_addr.sin_family = PF_INET; // /* IP protocol family.  */
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    server_addr.sin_port = htons(convert_string_to_long_int(port));

    for (int j = 0; j < ips.ips_size; j++)
    {

        //преобразовывает обычный вид IP-адреса cp (из номеров и точек) в двоичный код
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
        log_e("%s", "Error find address by domain name");
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
    log_i("resolve MX record for %s",name );
    u_char response[NS_PACKETSZ];
    ns_msg msg;
    ns_rr rr;
    int mx_index, ns_index, len;
    char dispbuf[4096];

    if ((len = res_search(name, C_IN, T_MX, response, NS_PACKETSZ + 1)) < 0)
    {
        log_e("Error find MX record for %s", name);
        return -1;
    }

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
        if (ns_parserr(&msg, ns_s_an, ns_index, &rr))
        {
            log_e("%s", "resolve_mx() ns_parserr()");
            continue;
        }

        ns_sprintrr(&msg, &rr, NULL, NULL, dispbuf, sizeof(dispbuf));
        if (ns_rr_class(rr) == ns_c_in && ns_rr_type(rr) == ns_t_mx)
        {
            char mxname[MAXDNAME];
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
