
#include "log.h"
#include "command.h"
#include "client_handler.h"
#include "maildir.h"
#include "socket_utils.h"

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
        }
    } else {
        if (client->state == STATE_DATA_RECEIVING) {
            append_body(client->letter, request);
            char* body_end = strstr(client->letter->body, "\r\n.\r\n");
            if (body_end != NULL) {
                client->state = STATE_DATA_RECEIVED;
                *body_end = '\0';
                
                result = process_letter(client);
            }
        } else {
            client->buffer = request;
            result = process_command(client);
        }
    }

    if (client->buffer != NULL) {
        free(client->buffer);
        client->buffer = NULL;
    }

    log_d("Client <%s> has been processed!", client->name);
    return result;
}

int process_command(client *client)
{
    char command[COMMAND_LENGTH + 1]; // 4 bytes for command + 1 for '\0'
    memcpy(command, client->buffer, COMMAND_LENGTH);
 
    switch (str_to_command(command))
    {
        case COMMAND_HELO: return handle_helo(client);       
        case COMMAND_EHLO: return handle_ehlo(client);
        case COMMAND_MAIL_FROM: return handle_mail_from(client);
        case COMMAND_RCPT_TO: return handle_rcpt_to(client);
        case COMMAND_DATA: return handle_data(client);
        case COMMAND_VRFY: return handle_vrfy(client);
        case COMMAND_RSET: return handle_rset(client);
        case COMMAND_QUIT:
            handle_quit(client);
            return FAIL;
        default: return send_response(client, STATUS_UNRECOGNIZED_COMMAND);
    }
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
    ssize_t message_size = BUFFER_SIZE;
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

        if (recv_bytes >= message_size)
        {
            size_t prev_size = message_size;
            message_size = recv_bytes + 1;
            void *new_message = malloc(message_size * sizeof(char));
            if (new_message == NULL) {
                free(message);
                log_e("Failed to read so big message from <%s> client (%d: %s).",
                    client->name, errno, strerror(errno));
                return NULL;
            }

            memcpy(new_message, message, prev_size * sizeof(char));
            free(message);
            message = new_message;
        }

        strcat(message, buffer);
        message[recv_bytes] = '\0';

        if (strstr(message, "\r\n") != NULL) {
            end = 1;
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

	if (bytes < 0) { // error while reading
        if (no_events()) {
            log_w("Failed to read from <%s> client (%d: %s).",
                client->name, errno, strerror(errno));
            if (strlen(message) > 0) {
                return message;
            }
            return NULL;
        } else {
            log_e("Failed to read from <%s> client (%d: %s).",
                client->name, errno, strerror(errno));
            client->state = STATE_CLOSED;
            return NULL;
        }
    }
	if (bytes == 0) { // connection is closed
        client->state = STATE_CLOSED;
        free(message);
		log_i("Client <%s> closed connection.", client->name);
		return NULL;
	}

    if (strlen(message) < MAX_COMMAND_LENGTH) {
        log_i("Client <%s>: %s", client->name, message);
    } else {
        log_i("Client <%s> sent a big message...", client->name);
    }
    return message;
}

char* get_mail(const char *message)
{
    char *start = strchr(message, '<') + 1;
    char *end = strchr(message, '>');
    char *at = strchr(message, '@');
    if (start == NULL || end == NULL || at == NULL) {
        return NULL;
    }

    int mail_size = end - start;
    if (mail_size < 3 || at - start < 1 || end - at - 1 < 1) {
        return NULL;
    }

    char *mail = malloc(sizeof(char) * (mail_size + 1));
    strncpy(mail, start, mail_size);
    mail[mail_size] = '\0';
    return mail;
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
    if (client->state != STATE_INIT) {
        log_w("Incorrect state for the client <%s>: expected %s, got %s.",
            client->name, state_to_str(STATE_INIT), state_to_str(client->state));
        return send_response(client, STATUS_IMPROPER_COMMAND_SEQUENCE);
    }

    char *mail = get_mail(client->buffer);
    if (mail == NULL) {
        log_w("Client <%s>: entered an incorrect email address \"%s\"",
            client->name, mail);
        return send_response(client, STATUS_INCORRECT_EMAIL);
    }

    log_i("Client <%s>: mail from \"%s\"", client->name, mail);
    client->letter = create_letter();
    client->letter->mail_from = mail;
    client->state = STATE_MAIL_STARTED;
    return send_response(client, STATUS_OK);
}

int handle_rcpt_to(client *client)
{
    if (client->state != STATE_MAIL_STARTED && client->state != STATE_RCPT_RECEIVED) {
        log_w("Incorrect state for the client <%s>: expected %s or %s, got %s.",
            client->name,
            state_to_str(STATE_MAIL_STARTED),
            state_to_str(STATE_RCPT_RECEIVED),
            state_to_str(client->state));
        return send_response(client, STATUS_IMPROPER_COMMAND_SEQUENCE);
    }
    
    char *mail = get_mail(client->buffer);
    if (mail == NULL) {
        log_w("Client <%s>: entered an incorrect email address \"%s\"",
            client->name, mail);
        return send_response(client, STATUS_INCORRECT_EMAIL);
    }
    
    log_i("Client <%s>: specified the recipient \"%s\"", client->name, mail)

    if(!add_recipient(client->letter, mail)) {
        return send_response(client, STATUS_TRANSACTION_FAILED);
    }

    client->state = STATE_RCPT_RECEIVED;
    return send_response(client, STATUS_OK);
}

int handle_data(client *client)
{
    if (client->state != STATE_RCPT_RECEIVED) {
        log_w("Incorrect state for the client <%s>: expected %s, got %s.",
            client->name, state_to_str(STATE_RCPT_RECEIVED), state_to_str(client->state));
        return send_response(client, STATUS_IMPROPER_COMMAND_SEQUENCE);
    }

    client->state = STATE_DATA_RECEIVING;
    return send_response(client, STATUS_START_INPUT);
}

int process_letter(client *client)
{
    char **filenames = (char**)malloc(client->letter->recipients_count * sizeof(char*));
    FILE **letter_files = (FILE**)malloc(client->letter->recipients_count * sizeof(FILE*));

    int created = 0;
    for (int i = 0; i < client->letter->recipients_count && i == created; i++) {
        filenames[i] = get_maildir_filename(client->letter->rcpt_domain[i]);
        if ((letter_files[i] = fopen(filenames[i], "a+")) == NULL) {
            log_e("Failed to create file '%s' (%d: %s).", filenames[i],
                errno, strerror(errno));
            send_response(client, STATUS_TRANSACTION_FAILED);
        } else {
            created++;
        }
    }
    
    if (created == client->letter->recipients_count) {
        send_response(client, STATUS_OK);
        for (int i = 0; i < client->letter->recipients_count; i++) {
            fprintf(letter_files[i], "From: %s\n", client->letter->mail_from);
            fprintf(letter_files[i], "To: ");
            for (int j = 0; j < client->letter->recipients_count; j++) {
                fprintf(letter_files[i], "%s", client->letter->rcpt_to[j]);
                if (j < client->letter->recipients_count - 1) {
                    fprintf(letter_files[i], ", ");
                }
            }
            fprintf(letter_files[i], "\n%s\n", client->letter->body);
        }
    }

    for(int i = 0; i < created; i++) {
        fclose(letter_files[i]);
        free(filenames[i]);
    }

    free(filenames);
    free(letter_files);
    free_letter(client->letter);
    client->letter = NULL;
    client->state = STATE_INIT;
    return SUCCESS;
}

int handle_vrfy(client *client)
{
    send_response(client, STATUS_NOT_IMPLEMENTED_COMMAND);
    return SUCCESS;
}

int handle_rset(client *client)
{
    free(client->buffer);
    client->buffer = NULL;
    free_letter(client->letter);
    client->letter = NULL;
    client->state = STATE_INIT;
    send_response(client, STATUS_OK);
    return SUCCESS;
}

int handle_quit(client *client)
{
    client->state = STATE_CLOSED;
    send_response(client, STATUS_CLOSING);
    return SUCCESS;
}
