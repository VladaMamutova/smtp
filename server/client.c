#include "client.h"

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h> // inet_ntoa

client* create_client(int socket, struct sockaddr_in address) {
    client *new_client = malloc(sizeof(client));
    new_client->socket = socket;
    new_client->address = address;
    sprintf(new_client->name, "%s:%d",
        inet_ntoa(address.sin_addr), address.sin_port);
    new_client->state = STATE_START;
    return new_client;
}

void free_client(client *client)
{
    if (client == NULL) {
        return;
    }
    free(client);
}
