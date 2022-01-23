#include "log.h"
#include "server.h"

#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in
#include <sys/ioctl.h>
#include <unistd.h>

#include <signal.h> // sigaction

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

poll_args server_poll = {0};

void run_server()
{
    init_signals_handler();
    
    server_poll.server_socket = -1;
    if (initSocket(&server_poll.server_socket) < 0) {
        log_e("%s", "Failed to create socket!");
        return;
    }
    
    memset(&server_poll.fds, 0, sizeof(server_poll.fds)); // Заполняем значениями по умолчанию
    
    server_poll.nfds = 1; // Изначально инициализируем структуру для одного клиента.
    server_poll.fds[0].fd = server_poll.server_socket;
    server_poll.fds[0].events = POLLIN; // готовность к чтению

    log_i("%s", "Server poll started started!");
    
    int result = start_poll(server_poll);
    if (result < 0) {
        log_e("%s", "Server poll failed");
    } else {
        log_i("%s", "Server poll finished successfully!");
    }

    log_i("%s", "Server process finished!");
    stop_server();
}

int initSocket(int *server_socket)
{
    int opt_val = 1;

    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket < 0) {
        printf("Error: socket() failed");
        return -1;
    }

    if (setsockopt(*server_socket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR,
                   (char *)&opt_val, sizeof(opt_val)) < 0) {
        printf("Error: setsockopt() failed");
        close(*server_socket);
        return -1;
    }

    if (ioctl(*server_socket, FIONBIO, (char *)&opt_val) < 0) {
        printf("Error: ioctl() failed");
        close(*server_socket);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT); // перевод в сетевой порядок байт
    if (bind(*server_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("Error: bind() failed\n");
        close(*server_socket);
        return -1;
    }

    if (listen(*server_socket, MAX_CLIENTS) < 0) {
        printf("Error: listen() failed\n");
        close(*server_socket);
        return -1;
    }
    return 0;
}

void init_signals_handler()
{
    signal(SIGINT, &handle_log_signal); // обработчик CTRL+C
}

void handle_server_signal(int signal)
{
    printf("Signal <SIGINT> received. Stopping server...\n");
    stop_server();
}

void stop_server() {
   for (int i = 0; i < server_poll.nfds; ++i) {
        if (server_poll.fds[i].fd >= 0) {
            close(server_poll.fds[i].fd);
        }
    }

    printf("Server process <%d> stopped!\n", getpid());
    exit(0);
}
