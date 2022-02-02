#ifndef STATUS_H
#define STATUS_H

#include <stdlib.h>
#include <stdio.h>

typedef enum {
    STATUS_SERVER_IS_READY = 220,
    STATUS_CLOSING = 221,
    STATUS_OK = 250,
    STATUS_NOT_VERIFIED = 252,
    STATUS_START_INPUT = 354,
    STATUS_SERVER_IS_NOT_AVAILABLE = 421,
    STATUS_ABORTED_DUE_TO_ERROR = 451,
    STATUS_UNRECOGNIZED_COMMAND = 500,
    STATUS_SYNTAX_ERROR_IN_ARGS = 501,
    STATUS_NOT_IMPLEMENTED_COMMAND = 502,
    STATUS_IMPROPER_COMMAND_SEQUENCE = 503,
    STATUS_INCORRECT_EMAIL = 553,
    STATUS_TRANSACTION_FAILED = 554
} status;


char* response_by_status(status code);

#endif // STATUS_H