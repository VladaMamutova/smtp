#ifndef _LOG_H_
#define _LOG_H_

#include <sys/msg.h> // ssize_t

typedef enum
{
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    DEBUG = 3
} log_level;

int start_logger(const char *filename);
int stop_logger();
log_level get_log_level();
void set_log_level(log_level log_level);
int log_message(log_level level, const char *message);
int save_message(const char *message, ssize_t size);
void handle_log_signal(int signal);

#define LOG(lvl, format_, ...) {                                    \
    int size = 1024;                                                \
    char *prefix;                                                   \
    if (lvl <= get_log_level()) {                                   \
        switch(lvl) {                                               \
        case INFO:                                                  \
            prefix = "INFO: ";                                      \
            break;                                                  \
        case WARN:                                                  \
            prefix = "WARN: ";                                      \
            break;                                                  \
        case ERROR:                                                 \
            prefix = "ERROR:";                                      \
            break;                                                  \
        case DEBUG:                                                 \
            prefix = "DEBUG:";                                      \
            break;                                                  \
        default:                                                    \
            abort();                                                \
        };                                                          \
        char *msg = malloc(size * sizeof(char));                    \
        snprintf(msg, size, "%s " format_"\n", prefix, __VA_ARGS__);\
        log_message(lvl, msg);                                      \
        free(msg);                                                  \
    }                                                               \
}

#define log_i(format, ...) LOG(INFO, format, __VA_ARGS__);
#define log_w(format, ...) LOG(WARN, format, __VA_ARGS__);
#define log_e(format, ...) LOG(ERROR, format, __VA_ARGS__);
#define log_d(format, ...) LOG(DEBUG, format, __VA_ARGS__);

#endif // _LOG_H_