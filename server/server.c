#include "log.h"
#include "server.h"
#include "config.h"
#include "maildir.h"
#include "socket_utils.h"

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
    init_maildir();

    int server_socket = -1;
    if (initSocket(&server_socket) < 0) {
        log_e("%s", "Failed to create socket!");
        return;
    }
    
    init_server_poll(&server_poll, server_socket);

    server_started = 1;
    log_i("%s", "Server poll started!\n");

    int result;
    do
    {
        result = do_poll(&server_poll);
    } while (result != -1);

    stop_server_poll(&server_poll);
    log_i("%s", "Server process finished!");
}

int initSocket(int *server_socket)
{
    int opt_val = 1;

    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket < 0) {
        printf("Error: socket() failed");
        return -1;
    }

    if (set_socket_nonblock(*server_socket) < 0) {
        printf("Error: Failed to set non-blocking mode for the server socket");
        close(*server_socket);
        return -1;
    }

    if (setsockopt(*server_socket, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR,
                   (char *)&opt_val, sizeof(opt_val)) < 0) {
        printf("Error: setsockopt() failed");
        close(*server_socket);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config_context.port); // перевод в сетевой порядок байт
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
    signal(SIGINT, &handle_server_signal); // обработчик CTRL+C
}

void handle_server_signal(int signal)
{
    printf("\nSignal <SIGINT> received. Stopping server...\n");
    server_started = 0;
}
