#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "client.h"

#define BUFFER_SIZE 1024

int greet_client(client *client);
int handle_client(client *client);

#endif // CLIENT_HANDLER_H
