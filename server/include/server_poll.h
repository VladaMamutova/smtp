#ifndef SERVER_POLL_H
#define SERVER_POLL_H

#include <sys/poll.h>

#define MAX_CLIENTS 200
#define BUFFER_SIZE 1024

typedef struct {
	int server_socket;
	struct pollfd fds[MAX_CLIENTS + 1]; // + 1 for the server socket
	int nfds;
} poll_args;

int do_poll(poll_args server_poll);
int is_server_socket(poll_args server_poll, int socket);
int handle_clients(poll_args *server_poll);
int accept_new_client(int server_socket);
int handle_client(int socket);
void remove_closed_clients(poll_args* server_poll);

#endif // SERVER_POLL_H
