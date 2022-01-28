#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

int set_socket_nonblock(int socket_fd);
int set_socket_timeout(int socket_fd, int seconds);
int no_connections();
int is_net_or_protocol_error();

#endif // SOCKET_UTILS_H
