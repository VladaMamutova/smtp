#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "client.h"
#include "status.h"

int greet_client(client *client);
int handle_client(client *client);
int process_command(client *client);
char* receive_request(client *client);
int send_response(client *client, status status);
char* get_mail(const char *message);
int process_letter(client *client);

int handle_helo(client *client);
int handle_ehlo(client *client);
int handle_mail_from(client *client);
int handle_rcpt_to(client *client);
int handle_data(client *client);
int handle_vrfy(client *client);
int handle_rset(client *client);
int handle_quit(client *client);

#endif // CLIENT_HANDLER_H
