#include "log.h"
#include "socket_utils.h"
#include "server_poll.h"

#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_ntoa
#include <unistd.h> // close
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // uint32_t
#include <string.h> // strerror
#include <errno.h>

void init_server_poll(poll_args *server_poll, int server_socket)
{
    server_poll->server_socket = server_socket;

    // Заполняем структуры для всех клиентов значениями по умолчанию.
    memset(&server_poll->fds, -1, sizeof(server_poll->fds));
    
    // Инициализируем структуру для прослушивающего серверного сокета.
    server_poll->nfds = 1;
    server_poll->fds[0].fd = server_poll->server_socket;
    server_poll->fds[0].events = POLLIN; // готовность к чтению

    // Инициализируем список с информацией о клиентах.
    init_client_hash(MAX_CLIENTS);
}

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
    for (int i = 0; i < client_number; ++i)
    {
        struct pollfd client = server_poll->fds[i];
        if (client.revents == 0) {
            continue;
        }

        if (!(client.revents & POLLIN)) { // обнуляем revents
            log_e("Incorrect events (%d) occured in the [%d] connection (%d).",
                  client.revents, i, client.fd);
            continue;
        }

        if (is_server_socket(*server_poll, client.fd)) {
            int accepted = accept_new_clients(server_poll);
            if (!accepted) {
                log_w("%s", "Unable to accept a new connection.");
            } else {
                log_i("Accepted %d clients.", accepted);
            }
        }
        else
        {
            int success = handle_client(client.fd);
            if (!success)
            {
                log_i("Close and release <%d> client.", client.fd);
                close(client.fd);
                remove_client(client.fd);
                client.fd = -1;
                closed++;
            }
        }
    }

    if (closed) {
        remove_closed_clients(server_poll);
        log_i("%d clients disconnected.", closed);
    }

    return 0;
}

int accept_new_clients(poll_args *server_poll)
{
    int accepted = 0;
    int error_attempts = 0;
    int try_again = 0;
    int socket;
    struct sockaddr_in address;
    uint32_t address_size = sizeof(address);
	do {
        log_d("%s", "Trying to accept a new connection...");
		socket = accept(server_poll->server_socket,
            (struct sockaddr*)&address, &address_size);
		if (socket < 0) {
            error_attempts++;
            log_e("Failed to accept new connection (%s).", strerror(errno));
            if (is_net_or_protocol_error()) {
                try_again = error_attempts <= ACCEPT_ATTEMPTS;
                log_d("Trying again: %s, attempt: %d",
                    try_again == 1 ? "true" : "false", error_attempts);
            } else {
                try_again = 0;
            }
		} else {
            error_attempts = 0;
            client *new_client = create_client(socket, address);
            insert_client(*new_client);

            log_i("New client %s:%d accepted (socket: %d).",
                inet_ntoa(address.sin_addr), address.sin_port, socket);

            set_socket_nonblock(socket);
            set_socket_timeout(socket, TIMEOUT);            

            server_poll->fds[server_poll->nfds].fd = socket;
            server_poll->fds[server_poll->nfds].events = POLLIN;
            ++server_poll->nfds;
            ++accepted;
        }
	} while (socket != -1 || try_again);

    return accepted;
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

void remove_closed_clients(poll_args *server_poll)
{
    for (int i = 0; i < server_poll->nfds; ++i) {
        if (server_poll->fds[i].fd == -1) {
            for (int j = i; j < server_poll->nfds; ++j) {
                server_poll->fds[j].fd = server_poll->fds[j + 1].fd;
            }
            --i;
            --server_poll->nfds;
        }
    }
}
