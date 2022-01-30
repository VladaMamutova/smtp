#ifndef COMMAND_H
#define COMMAND_H

#include <string.h>

#define COMMAND_LENGTH 4
#define COMMAND_NUMBER 8

typedef enum {
    COMMAND_UNKNOWN = -1,

    COMMAND_HELO = 0,
    COMMAND_EHLO = 1,
    COMMAND_MAIL_FROM = 2,
    COMMAND_RCPT_TO = 3,
    COMMAND_DATA = 4,
    COMMAND_VRFY = 5,
    COMMAND_RSET = 6,
    COMMAND_QUIT = 7
    
} command;

static const char commands[COMMAND_NUMBER][COMMAND_LENGTH + 1] = {
    "HELO", // 0
    "EHLO", // 1
    "MAIL", // 2
    "RCPT", // 3
    "DATA", // 4
    "VRFY", // 5
    "RSET", // 6
    "QUIT"  // 7
};

command str_to_command(const char command_str[])
{
    if (strlen(command_str) != COMMAND_LENGTH) { // '\0' не входит в длину строки
        return COMMAND_UNKNOWN;
    }

    for (int i = 0; i < COMMAND_NUMBER; ++i)
    {
        if (strcasecmp(command_str, commands[i]) == 0) { // без учёта регистра
            return (command)i;
        }
    }

    return COMMAND_UNKNOWN;
}

#endif // COMMAND_H
