#include "log.h"
#include "server_poll.h"

#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h> // close
#include <stdio.h>
#include <stdlib.h>

#include <string.h> // strerror
#include <errno.h>

int do_poll(poll_args server_poll)
{
    log_i("%s", "Waiting for clients...");

    int ready = poll(server_poll.fds, (nfds_t)server_poll.nfds, -1); // -1 - infinite timeout
    if (ready < -1) {
        log_e("Failed to poll clients (%s).", strerror(errno));
        return -1;
    }

    log_i("Ready clients: %d.\n", ready);

    handle_clients(&server_poll);

    return 1;
}

int is_server_socket(poll_args server_poll, int socket)
{
    return socket == server_poll.server_socket;
}

int handle_clients(poll_args *server_poll)
{
    int closed = 0;
    int client_number = server_poll->nfds;
    for (int i = 0; i < client_number; i++)
    {
        struct pollfd client = server_poll->fds[i];
        if (client.revents == 0) {
            continue;
        }

        if (!(client.revents & POLLIN)) {
            log_e("Incorrect events (%d) occured in the [%d] connection (%d).",
                  client.revents, i, client.fd);
            return -1; // continue?
        }

        if (is_server_socket(*server_poll, client.fd)) {
            int accepted = accept_new_client(client.fd);
            if (!accepted) {
                log_w("%s", "Unable to accept a new connection.");
                continue;
            }
        }
        else
        {
            int success = handle_client(client.fd);
            if (!success)
            {
                log_i("Close and release <%d> client.", client.fd);
                close(client.fd);
                client.fd = -1;
                closed++;
            }
        }
    }

    if (closed) {
        remove_closed_clients(server_poll);
    }

    return 0;
}

int accept_new_client(int server_socket)
{
    // TODO
    return 0;
}

int handle_client(int socket)
{
	char buffer[BUFFER_SIZE];
    int bytes;
	do {
		bytes = recv(socket, buffer, sizeof(buffer), 0);
		if (bytes < 0) { // error while reading
			log_i("Failed to read from <%d> client (%s).", socket, strerror(errno));
			return -1;
        }
		if (bytes == 0) { // connection is closed
			log_i("Client <%d> closed connection.", socket);
			return -1;
		}
	} while (bytes);

    // TODO: обработать принятое сообщение
	return 1;
}

void remove_closed_clients(poll_args* server_poll)
{
    // TODO: учесть новые подключения
    for (int i = 0; i < server_poll->nfds; i++) { // + 1 на новый клиент ?
        if (server_poll->fds[i].fd == -1) {
            for (int j = i; j < server_poll->nfds; ++j) { // + 1 на новый клиент ?
                server_poll->fds[j].fd = server_poll->fds[j + 1].fd;
            }
            i--;
            server_poll->nfds--;
        }
    }
}
