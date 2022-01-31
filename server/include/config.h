#ifndef CONFIG_H
#define CONFIG_H

#include <libconfig.h>

#define SERVER_CONFIG "server.cfg"

typedef struct config {
    char *maildir;
    char *domain;
    int port;
    char *log_file;
    int log_level;
} config;

extern config config_context;

int load_config();
void free_config();

#endif
