#include "socket_utils.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>

int set_socket_nonblock(int socket_fd)
{
    int flags;
    if ((flags = fcntl(socket_fd, F_GETFL, NULL)) < 0) {
        return -1;
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        return -1;
    }
    return 0;
}

int set_socket_timeout(int socket_fd, int seconds)
{
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    return setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

int no_events()
{
    // Сокет помечен как неблокирующий, но запрашиваемая операция будет заблокирована
    // (в очереди нет запросов на соединение, нет данных для чтения, для записи)
    return errno == EWOULDBLOCK || errno == EAGAIN;
}

int is_net_or_protocol_error()
{
    return errno == ENETDOWN || errno == EPROTO || 
           errno == ENOPROTOOPT || errno == EHOSTDOWN ||
           errno == ENONET || errno == EHOSTUNREACH || 
           errno == EOPNOTSUPP || errno == ENETUNREACH;
}
