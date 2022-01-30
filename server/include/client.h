#ifndef CLIENT_H
#define CLIENT_H

#include "state.h"

#include <netinet/in.h> // sockaddr_in

#define NAME_SIZE 22 // ip:port -> xxx.xxx.xxx.xxx:yyyyy -> 21 + 1 for '\0'

typedef struct {
    int socket;
    struct sockaddr_in address;
    char name[NAME_SIZE];
	state state;
} client;

client *create_client(int socket, struct sockaddr_in address);
char* client_name(client *client);
void free_client(client *client);

#endif // CLIENT_H
