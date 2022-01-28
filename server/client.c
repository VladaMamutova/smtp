#include "client.h"

#include <stdlib.h>

client* create_client(int socket, struct sockaddr_in address) {
    client *new_client = malloc(sizeof(client));
    new_client->socket = socket;
    new_client->address = address;
    new_client->send_buf = calloc(BUFFER_SIZE, sizeof(char));
    new_client->recv_buf = calloc(BUFFER_SIZE, sizeof(char));
    return new_client;
}

void free_client(client *client)
{
    if (client == NULL) {
        return;
    }
    free(client->send_buf);
    free(client->recv_buf);
    free(client);
}
