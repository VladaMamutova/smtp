#include "log.h"
#include "socket_utils.h"
#include "server_poll.h"
#include "client_handler.h"

#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <unistd.h> // close
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // uint32_t
#include <string.h> // strerror
#include <errno.h>

int server_started = 0;

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

int do_poll(poll_args *server_poll)
{
    log_d("%s", "Waiting for client actions...");

    int ready = poll(server_poll->fds, (nfds_t)server_poll->nfds, -1); // -1 - infinite timeout
    if (ready < -1) {
        if (!server_started) {
            stop_server_poll(server_poll);
        } else {
            log_e("Failed to poll clients (%d: %s).", errno, strerror(errno));
            return -1;
        }
    }

    process_ready_clients(server_poll);

    log_d("%s\n", "All clients processed!");
    return 0;
}

int is_server_socket(poll_args server_poll, int socket)
{
    return socket == server_poll.server_socket;
}

void process_ready_clients(poll_args *server_poll)
{
    int closed = 0;
    for (int i = 0; i < server_poll->nfds; ++i)
    {
        if (!server_started) {
            stop_server_poll(server_poll);
        }
        struct pollfd client = server_poll->fds[i];
        if (client.revents <= 0) {
            continue;
        }

        if (!(client.revents & POLLIN)) { // проверяем событие чтения в revents
            log_e("Incorrect events (%d) occured in the [%d]-connection (socket: %d).",
                  client.revents, i, client.fd);
            continue;
        }

        if (is_server_socket(*server_poll, client.fd)) {
            int accepted = accept_new_client(server_poll);
            if (!accepted) {
                log_w("%s", "Unable to accept a new connection.");
            }
        }
        else
        {
            client_hash *client_hash = find_client(client.fd);
            int success = handle_client(client_hash->client_info);
            if (!success)
            {
                log_i("Close and release client <%s> (socket: %d).",
                    client_hash->client_info->name, client.fd);
                close(server_poll->fds[i].fd);
                remove_client(client.fd);
                // Устанавливаем признак отключения клиента -1 (у объекта в массиве!).
                server_poll->fds[i].fd = -1;
                closed++;
            }
        }
    }

    if (closed) {
        remove_closed_clients(server_poll);
        log_i("%d clients disconnected.", closed);
    }
}

int accept_new_client(poll_args *server_poll)
{
    log_d("%s", "Trying to accept a new connection...");

    int attempts = 1;
    int try_again = 0;
    int socket;
    struct sockaddr_in address;
    uint32_t address_size = sizeof(address);
	do {
        if (attempts > 1) {
            log_d("%d attempt to accept a new connection...", attempts);
        }
		socket = accept(server_poll->server_socket,
            (struct sockaddr*)&address, &address_size);
		if (socket == -1) {
            log_e("Failed to accept a new connection (%d: %s).", errno, strerror(errno));
            if (no_events() || is_net_or_protocol_error()) {
                try_again = ++attempts <= ACCEPT_ATTEMPTS;
            }
		} else {
            set_socket_nonblock(socket);
            set_socket_timeout(socket, TIMEOUT);

            client *new_client = create_client(socket, address);
            log_i("A new incoming connection from <%s> (socket: %d) has been received.",
                new_client->name, socket);

            if (greet_client(new_client)) {
                log_i("New client <%s> (socket: %d) accepted.",
                    new_client->name, socket);

                insert_client(new_client);
                server_poll->fds[server_poll->nfds].fd = socket;
                server_poll->fds[server_poll->nfds].events = POLLIN;
                server_poll->fds[server_poll->nfds].revents = -1;
                ++server_poll->nfds;
            }
        }
	} while (try_again);

    return socket != -1;
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

void stop_server_poll(poll_args *server_poll)
{
    for (int i = 1; i < server_poll->nfds; ++i) {
       int client = server_poll->fds[i].fd;
        if (client >= 0) {
            client_hash *client_hash = find_client(client);
            send_response(client_hash->client_info, STATUS_SERVER_IS_NOT_AVAILABLE);
            log_i("Close and release client <%s> (socket: %d).",
                client_hash->client_info->name, client);
            remove_client(client);
            close(client);            
        }
    }

    close(server_poll->fds[0].fd);

    printf("Server process <%d> stopped!\n", getpid());
    exit(1);
}
