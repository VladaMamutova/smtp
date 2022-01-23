#include <string.h>
#include <stdlib.h>
#include "util.h"

string* add_character(string *str, char character);

long int convert_string_to_long_int(const char *str) {
    char *end;
    return strtol(str, &end, 10);
}

int check_equation_delim(const char *const str, const char *const delim, int offset) {
    int i = 0;
    while (str[offset] == delim[i]) {
        offset++;
        i++;
    }

    if (i == strlen(delim)) {
        return offset;
    }

    return -1;
}

string_tokens split(const char *const str, const char *const delim) {

    int count_tokens = 0;
    string *tokens = allocate_memory((sizeof(string)));

    string *token = allocate_memory(sizeof(string));
    token->chars = NULL;
    token->length = 0;

    for (int i = 0; i < strlen(str) + 1; i++) {
        int offset;
        if ((offset = check_equation_delim(str, delim, i)) != -1 || str[i] == '\0') {
            if (offset != -1) {
                i = offset - 1;
            }
            if (token->length != 0) {
                tokens[count_tokens] = *token;
                free(token);
                count_tokens++;
                token = allocate_memory(sizeof(*token));
                token->chars = NULL;
                token->length = 0;
                tokens = reallocate_memory(tokens,sizeof(string) * count_tokens,
                                           sizeof(string) * (count_tokens + 1));
                continue;
            }
        }
        add_character(token, str[i]);
    }
    free(token);

    string_tokens string_tokens;
    string_tokens.tokens = tokens;
    string_tokens.count_tokens = count_tokens;

    return string_tokens;
}

void free_string_tokens(string_tokens *tokens) {
    for (int i = 0; i < tokens->count_tokens; i++) {
        free(tokens->tokens[i].chars);
    }
    free(tokens->tokens);
}

string *add_character(string *str, char character) {
    size_t capacity = str->length + 1;
    if (str->chars == NULL) {
        str->length = 0;
        str->chars = allocate_memory(sizeof(char));
    }

    str->chars[(str->length)++] = character;

    if (str->length >= capacity) {
        size_t prev_size = capacity;
        capacity *= 2; // увеличиваем ёмкость строки в два раза
        str->chars = reallocate_memory(str->chars, prev_size, sizeof(char) * capacity); // создаём новую строку с увеличенной ёмкостью
    }

    str->chars[str->length] = '\0';

    return str;
}

void free_string(string *str) {
    free(str->chars);
    free(str);
}

void *allocate_memory(size_t bytes) {
    void *buffer = malloc(bytes);

    if (buffer != NULL) {
        return buffer;
    }

    exit(-1);
}

void *reallocate_memory(void *buffer, size_t prev_size, size_t new_size) {
    void *new_buffer = malloc(new_size);

    if (new_buffer != NULL) {
        memcpy(new_buffer, buffer, prev_size);
        free(buffer);
        return new_buffer;
    }

    exit(-1);
}

void *callocate_memory(size_t size, size_t bytes) {
    void *buffer = calloc(size, bytes);

    if (buffer != NULL) {
        return buffer;
    }

    exit(-1);
}

void trim(char *str) {
    {
        // удаляем пробелы и табы с начала строки:
        size_t i = 0, j;
        while ((str[i] == ' ') || (str[i] == '\t')) {
            i++;
        }
        if (i > 0) {
            for (j = 0; j < strlen(str); j++) {
                str[j] = str[j + i];
            }
            str[j] = '\0';
        }

        // удаляем пробелы и табы с конца строки:
        i = strlen(str) - 1;
        while ((str[i] == ' ') || (str[i] == '\t')) {
            i--;
        }
        if (i < (strlen(str) - 1)) {
            str[i + 1] = '\0';
        }
    }
}

char* file_readline(FILE *fp) {
    char read_string[256] = { 0 };
    size_t offset = 0;
    size_t size = 256;

    char *string = malloc(sizeof(char) * 256);
    while ((fgets(read_string, 256, fp)) != NULL) {
        if (offset >= size - 1) {
            size_t prev_size = size;
            size = offset * 2 + 1;
            string = reallocate_memory(string, sizeof(char) * prev_size, sizeof(char) * size);
        }

        strcpy(string + offset, read_string);

        offset += strlen(read_string);

        if (string[offset - 1] == '\n') {
            return string;
        }
    }
    free(string);
    return NULL;
}
