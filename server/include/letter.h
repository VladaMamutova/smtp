#ifndef _LETTER_H_
#define _LETTER_H_

#include <stdio.h>

#define MAX_RECIPIENTS 20

typedef struct
{
    char *mail_from;
    char **rcpt_to;
    char **rcpt_domain;
    int recipients_count;
    char* body;
} letter;

letter *create_letter();
int add_recipient(letter *letter, char* mail);
void append_body(letter *letter, const char *content);
void free_letter(letter *letter);

#endif // _LETTER_H_
