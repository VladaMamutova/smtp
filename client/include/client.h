#ifndef CLIENT_H
#define CLIENT_H

#include <poll.h>
#include "directory.h"

typedef struct context
{
    struct pollfd *poll_fds;
    int conn_context_count;
    maildir_main *maildir;
    int map[MAX_MX_ADDRS];
} context;

void run_client();
void handle_signal(int signal);
void handle_connect(int index);
void handler_helo(int index);
void handler_mail(int index);
void handler_rcpt(int index);
void handler_data(int index);
void handler_end_message(int index);
char *prepare_message(maildir_other_server *server);
void remove_server_from_context(int index);
status_code get_response(maildir_other_server *server);
void free_server(maildir_other_server *server);
void free_maildir(maildir_main *maildir);
void free_context();

#endif