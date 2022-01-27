#include "socket_utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

int set_socket_nonblock(int socket_fd) {
    int flags;
    if ((flags = fcntl(socket_fd, F_GETFL, NULL)) < 0) {
        return -1;
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        return -1;
    }
    return 0;
}

int no_connection() {
    return errno == EWOULDBLOCK || errno == EAGAIN; // сокет неблокирующий и в очереди нет запросов на соединение
}

int is_net_or_protocol_error() {
    return errno == ENETDOWN || errno == EPROTO || 
           errno == ENOPROTOOPT || errno == EHOSTDOWN ||
           errno == ENONET || errno == EHOSTUNREACH || 
           errno == EOPNOTSUPP || errno == ENETUNREACH;
}
