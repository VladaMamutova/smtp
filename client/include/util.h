#ifndef UTIL_H
#define UTIL_H

#include <sys/queue.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define strsize(args...) snprintf(NULL, 0, args) + sizeof('\0')
#define vstrsize(args...) snprintf(NULL, 0, args) + sizeof('\0')

typedef struct my_pair {
    char *first;
    char *second;
} my_pair;

typedef struct my_int_pair {
    int in_dir;
    int in_fd;
} my_int_pair;

typedef struct my_str {
    char *chars;
    size_t length;
} my_str;

typedef struct string_tokens {
    my_str *tokens;
    size_t count_tokens;
} string_tokens;

long int convert_string_to_long_int(const char *str);

string_tokens split(const char *const str, const char *const delim);
void free_string_tokens(string_tokens *tokens);

void free_string(my_str *str);
void trim(char *str);

void* allocate_memory(size_t bytes);
void* reallocate_memory(void* buffer, size_t prev_size, size_t new_size);
void* callocate_memory(size_t size, size_t bytes);

char* file_readline(FILE *fp);

bool is_interrupt();

#endif