#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h> // sockaddr_in

#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    struct sockaddr_in address;
    char* send_buf;
    char* recv_buf;

	int state;
    int buffer;
} client;

client *create_client(int socket, struct sockaddr_in address);
void free_client(client *client);

#endif // CLIENT_H
