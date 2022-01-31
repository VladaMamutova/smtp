#include "status.h"
#include "config.h"
#include <string.h>

char* response_by_status(status code)
{
    char* response = malloc(100);
    char message[90];
    switch (code)
    {
    case STATUS_SERVER_IS_READY: 
        strcpy(message, config_context.domain);
        break;
    case STATUS_CLOSING:
        strcpy(message, "Closing connection");
        break;
    case STATUS_OK:
        strcpy(message, "Ok");
        break;
    case STATUS_NOT_VERIFIED:
        strcpy(message, "Cannot verify the user, but the message will be accepted and attempted for delivery");
        break;
    case STATUS_START_INPUT:
        strcpy(message, "Start mail input (end with <CRLF>.<CRLF>)");
        break;
    case STATUS_SERVER_IS_NOT_AVAILABLE:
        strcpy(message, "The server is not available");
        break;
    case STATUS_ABORTED_DUE_TO_ERROR:
        strcpy(message, "Aborted the command due to a local error");
        break;
    case STATUS_UNRECOGNIZED_COMMAND:
        strcpy(message, "Cannot recognize the command due to syntax error");
        break;
    case STATUS_SYNTAX_ERROR_IN_ARGS:
        strcpy(message, "Syntax error in parameters or arguments");
        break;
    case STATUS_NOT_IMPLEMENTED_COMMAND:
        strcpy(message, "The command is not implemented");
        break;
    case STATUS_IMPROPER_COMMAND_SEQUENCE:
        strcpy(message, "Improper sequence of commands");
        break;
    case STATUS_INCORRECT_EMAIL:
        strcpy(message, "Syntactically incorrect mail address");
        break;
    case STATUS_TRANSACTION_FAILED:
        strcpy(message, "The transaction failed due to an unknown error");
        break;
    default:
        strcpy(message, "Unknown status code");
        break;
    }
    
    sprintf(response, "%d %s\r\n", code, message);
    return response;
}
