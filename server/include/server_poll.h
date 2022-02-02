#ifndef SERVER_POLL_H
#define SERVER_POLL_H

#include "client_hash.h"

#include <sys/poll.h>

#define MAX_CLIENTS 1024
#define TIMEOUT 1000
#define ACCEPT_ATTEMPTS 3

typedef struct {
	int server_socket;
	struct pollfd fds[MAX_CLIENTS + 1]; // + 1 for the server socket
	int nfds;
} poll_args;

extern int server_started;

void init_server_poll(poll_args *server_poll, int server_socket);
int do_poll(poll_args *server_poll);
int is_server_socket(poll_args server_poll, int socket);
void process_ready_clients(poll_args *server_poll);
int accept_new_client(poll_args *server_poll);
void remove_closed_clients(poll_args* server_poll);
void stop_server_poll(poll_args *server_poll);

#endif // SERVER_POLL_H
