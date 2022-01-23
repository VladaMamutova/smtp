#ifndef SERVER_POLL_H
#define SERVER_POLL_H

#include <sys/poll.h>

#define MAX_CLIENTS 200

typedef struct {
	int id;
	int server_socket;
	struct pollfd fds[MAX_CLIENTS]; // +1 ?
	int nfds;
} poll_args;

int start_poll(poll_args server_poll);

#endif // SERVER_POLL_H
