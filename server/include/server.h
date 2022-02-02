#ifndef SERVER_H
#define SERVER_H

#include "server_poll.h"

extern poll_args server_poll;

void run_server();
int initSocket(int *server_socket);
void init_signals_handler();
void handle_server_signal(int signal);
void stop_server();

#endif // SERVER_H
