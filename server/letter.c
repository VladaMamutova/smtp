#include "letter.h"

#include <stdlib.h>
#include <string.h>

letter *create_letter()
{
    letter *new_letter = malloc(sizeof(letter));
    new_letter->mail_from = NULL;
    new_letter->rcpt_to = (char**)malloc(MAX_RECIPIENTS * sizeof(char*));
    new_letter->rcpt_domain = (char **)malloc(MAX_RECIPIENTS * sizeof(char*));
    new_letter->recipients_count = 0;

    new_letter->body = NULL;
    return new_letter;
}

int add_recipient(letter *letter, char* mail)
{
    if (letter->recipients_count >= MAX_RECIPIENTS) {
        return 0;
    }

    letter->rcpt_to[letter->recipients_count] = mail;
    char* domain = strchr(mail, '@') + 1;
    int domain_size = strlen(mail) - (domain - mail) + 1;
    letter->rcpt_domain[letter->recipients_count] =
        malloc(domain_size * sizeof(char));
    strncpy(letter->rcpt_domain[letter->recipients_count],
        domain, domain_size);
    letter->recipients_count++;

    return letter->recipients_count;
}

void append_body(letter *letter, const char* content) {
    if (letter->body == NULL) {
        letter->body = (char *)calloc(strlen(content) + 1, sizeof(char));
        strncpy(letter->body, content, strlen(content));
    } else {
        char *body = calloc(strlen(letter->body) + strlen(content) + 1, sizeof(char));
        strcat(body, letter->body);
        strcat(body, content);
        free(letter->body);
        letter->body = body;
    }
}

void free_letter(letter *letter)
{
    if (letter == NULL) {
        return;
    }

    if (letter->mail_from != NULL) {
        free(letter->mail_from);
        letter->mail_from = NULL;
    }

    if (letter->rcpt_to != NULL) {
        for (int i = 0; i < letter->recipients_count; ++i) {
            free(letter->rcpt_to[i]);
        }
        free(letter->rcpt_to);
        letter->rcpt_to = NULL;
    }

    if (letter->rcpt_domain != NULL) {
        for (int i = 0; i < letter->recipients_count; ++i) {
            free(letter->rcpt_domain[i]);
        }
        free(letter->rcpt_domain);
        letter->rcpt_domain = NULL;
    }

    if (letter->body != NULL) {
        free(letter->body);
        letter->body = NULL;
    }

    free(letter);
}
