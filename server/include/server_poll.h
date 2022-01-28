#ifndef SERVER_POLL_H
#define SERVER_POLL_H

#include "client_hash.h"

#include <sys/poll.h>

#define MAX_CLIENTS 1024
#define TIMEOUT 1000
#define ACCEPT_ATTEMPTS 2

typedef struct {
	int server_socket;
	struct pollfd fds[MAX_CLIENTS + 1]; // + 1 for the server socket
	int nfds;
} poll_args;

void init_server_poll();
int do_poll(poll_args server_poll);
int is_server_socket(poll_args server_poll, int socket);
int handle_clients(poll_args *server_poll);
int accept_new_clients(poll_args *server_poll);
int handle_client(int socket);
void remove_closed_clients(poll_args* server_poll);

#endif // SERVER_POLL_H
