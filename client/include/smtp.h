#ifndef SMTP_H
#define SMTP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#define MAX_MX_ADDRS 30

typedef enum smtp_state_code {
    CONNECT = 0,
    HELO = 1,
    EHLO = 2,
    MAIL = 3,
    RCPT = 4,
    DATA = 5,
    MESSAGE = 6,
    END_MESSAGE = 7,
    RSET = 8,
    QUIT = 9,
    INVALID
} state_code;

typedef struct smtp_context {
    int socket_desc;
    state_code state_code;
} smtp_context;

typedef enum smtp_status_code {
    SMTP_COMMAND_FOR_HUMAN = 214,
    SMTP_READY_FOR_WORK = 220,
    SMTP_CLOSING = 221,
    SMTP_SUCCESS = 250,
    SMTP_RECIPIENT_IS_NOT_LOCAL = 251,
    SMTP_NOT_VERIFIED = 252,
    SMTP_CONTEXT_INPUT = 354,
    SMTP_SERVER_IS_NOT_AVAILABLE = 421,
    SMTP_MAX_SIZE_MESSAGE = 422,
    SMTP_DISCONNECTION_DURING_TRANSMISSION = 442,
    SMTP_REQUESTED_COMMAND_FAILED = 450,
    SMTP_COMMAND_ABORTED_SERVER_ERROR = 451,
    SMTP_COMMAND_ABORTED_INSUFFICIENT_SYSTEM_STORAGE = 452,
    SMTP_SERVER_COULD_NOT_RECOGNIZE_COMMAND = 500,
    SMTP_SYNTAX_ERROR_COMMAND_ARGS = 501,
    SMTP_COMMAND_IS_NOT_IMPLEMENTED = 502,
    SMTP_SERVER_HAS_ENCOUNTERED_A_BAD_SEQUENCE_OF_COMMANDS = 503,
    SMTP_COMMAND_PARAMETER_IS_NOT_IMPLEMENTED = 504,
    SMTP_HOST_NEVER_ACCEPTS_MAIL = 521,
    SMTP_CONTEXT_COULD_NOT_BE_DELIVERED_FOR_POLICY_REASONS = 541,
    SMTP_USER_MAILBOX_WAS_UNAVAILABLE = 550,
    SMTP_RECIPIENT_IS_NOT_LOCAL_TO_THE_SERVER = 551,
    SMTP_ACTION_WAS_ABORTED_DUE_TO_EXCEEDED_STORAGE_ALLOCATION = 552,
    SMTP_MAILBOX_NAME_IS_INVALID = 553,
    SMTP_MAILBOX_DISABLED = 554,
    UNDEFINED_ERROR = 0,
    NOT_ANSWER = 1
} status_code;

typedef struct smtp_response {
    status_code status_code;
    char *message;
} smtp_response;


typedef struct ips {
    char **ip_array;
    size_t ips_size;
} ips;

int connect_to_server(ips ips, char *port);
smtp_context *smtp_connect(char *server, char *port);
ips get_ips_by_hostname(char *hostname);
int resolve_mx(const char *name, char **mxs, int limit);
char *get_addr_by_socket(int socket);
char *read_from_socket(int socket_d);
smtp_response get_smtp_response(smtp_context *context);
state_code smtp_send_helo(smtp_context *context);
state_code smtp_send_mail(smtp_context *context, char *from);
state_code smtp_send_rcpt(smtp_context *context, char *to); 
state_code smtp_send_data(smtp_context *context);
state_code smtp_send_message(smtp_context *context, char *message);
state_code smtp_send_end_message(smtp_context *context);
state_code smtp_send_quit(smtp_context *context); 
int is_smtp_success(status_code status_code);
int is_smtp_4yz_code(status_code status_code);
int is_smtp_5yz_code(status_code status_code);
#endif 