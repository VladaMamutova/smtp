
#include "log.h"
#include "command.h"
#include "client_handler.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SUCCESS 1
#define FAIL 0

int greet_client(client *client)
{
    if (!send_response(client, STATUS_SERVER_IS_READY)) {
        client->state = STATE_CLOSED;
        return FAIL;
    }

    client->state = STATE_START;
    return SUCCESS;
}

int handle_client(client *client)
{
    log_d("Processing the client <%s>...", client->name);

    int result = SUCCESS;
    char* request = receive_request(client);
    if (request == NULL) {
        if (client->state == STATE_CLOSED) {
            result = FAIL;
        } else if (!send_response(client, STATUS_ABORTED_DUE_TO_ERROR)) {
            client->state = STATE_CLOSED;
            result = FAIL;
        }
    } else {
        char command[COMMAND_LENGTH + 1]; // 4 bytes for command + 1 for '\0'
        memcpy(command, request, COMMAND_LENGTH);

        switch (str_to_command(command))
        {
        case COMMAND_HELO:
            result = handle_helo(client);
            break;
        case COMMAND_EHLO:
            result = handle_ehlo(client);
            break;
        case COMMAND_MAIL_FROM:
            result = handle_mail_from(client);
            break;
        case COMMAND_RCPT_TO:
            result = handle_rcpt_to(client);
            break;
        case COMMAND_DATA:
            result = handle_data(client);
            break;
        case COMMAND_VRFY:
            result = handle_vrfy(client);
            break;
        case COMMAND_RSET:
            result = handle_rset(client);
            break;
        case COMMAND_QUIT:
            handle_quit(client);
            result = FAIL;
            break;
        default:
            send_response(client, STATUS_UNRECOGNIZED_COMMAND);
            break;
        }
    }

    log_d("Client <%s> has been processed!", client->name);
    return result;
}

int send_response(client *client, status status)
{
    char *response = response_by_status(status);
    int result = send(client->socket, response, strlen(response), 0);
    if (result > 0) {
        log_i("Sent response to the client <%s>:\n%s%s",
            client->name, "                           ", response);
    } else {
        log_e("Failed to send response to the client <%s> (%d: %s).",
            client->name, errno, strerror(errno));
    }
    free(response);
    return result > 0;
}

char* receive_request(client *client)
{
    ssize_t message_size = BUFFER_SIZE + 1; // + 1 for '\0'
    char *message = calloc(message_size, sizeof(char));
    message[0] = '\0';

    ssize_t recv_bytes = 0;

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int end = 0;
    ssize_t bytes;
    while (!end && (bytes = recv(client->socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        recv_bytes += bytes;
        if (recv_bytes > message_size) {
            message_size = recv_bytes + 1; // + 1 for '\0'
            message = (char *)realloc(message, message_size);
        }

        strcat(message, buffer);

        if (strstr(message, "\r\n") != NULL) {
            end = 1;
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

	if (bytes < 0) { // error while reading
        free(message);
		log_e("Failed to read from <%s> client (%d: %s).",
            client->name, errno, strerror(errno));
		return NULL;
    }
	if (bytes == 0) { // connection is closed
        client->state = STATE_CLOSED;
        free(message);
		log_i("Client <%s> closed connection.", client->name);
		return NULL;
	}

    return message;
}

int handle_helo(client *client)
{
    if (client->state != STATE_START) {
        log_w("Incorrect state for the client <%s>: expected %s, got %s.",
            client->name, state_to_str(STATE_START), state_to_str(client->state));
        return send_response(client, STATUS_IMPROPER_COMMAND_SEQUENCE);
    }

    if (!send_response(client, STATUS_OK)) {
        return FAIL;
    }

    client->state = STATE_INIT;
    return SUCCESS;
}

int handle_ehlo(client *client)
{
    return handle_helo(client);
}

int handle_mail_from(client *client)
{
    return SUCCESS;
}

int handle_rcpt_to(client *client)
{
    return SUCCESS;
}

int handle_data(client *client)
{
    return SUCCESS;
}

int handle_vrfy(client *client)
{
    send_response(client, STATUS_NOT_IMPLEMENTED_COMMAND);
    return SUCCESS;
}

int handle_rset(client *client)
{
    client->state = STATE_START;
    send_response(client, STATUS_OK);
    return SUCCESS;
}

int handle_quit(client *client)
{
    client->state = STATE_CLOSED;
    send_response(client, STATUS_CLOSING);
    return SUCCESS;
}
