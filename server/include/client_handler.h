#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "client.h"
#include "status.h"

#define BUFFER_SIZE 1024

int greet_client(client *client);
int handle_client(client *client);
char* receive_request(client *client);
int send_response(client *client, status status);

int handle_helo(client *client);
int handle_ehlo(client *client);
int handle_mail_from(client *client);
int handle_rcpt_to(client *client);
int handle_data(client *client);
int handle_vrfy(client *client);
int handle_rset(client *client);
int handle_quit(client *client);

#endif // CLIENT_HANDLER_H
