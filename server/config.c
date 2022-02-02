#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

config config_context = { 0 };

int load_config() {
    config_t cfg;
    config_init(&cfg);

    if (!(config_read_file(&cfg, SERVER_CONFIG))) {
        config_destroy(&cfg);
        return 0;
    }

    const char *maildir = NULL;
    if (!config_lookup_string(&cfg, "maildir", &maildir)) {
        printf("Failed to read \"maildir\" attribute from config '%s'", SERVER_CONFIG);
        config_destroy(&cfg);
        return 0;
    }

    const char *domain = NULL;
    if (!config_lookup_string(&cfg, "domain", &domain)) {
        printf("Failed to read \"domain\" attribute from config '%s'", SERVER_CONFIG);
        config_destroy(&cfg);
        return 0;
    }

    int port = 0;
    if (!config_lookup_int(&cfg, "port", &port)) {
        printf("Failed to read \"port\" attribute from config '%s'", SERVER_CONFIG);
        config_destroy(&cfg);
        return 0;
    }

    const char *log_file = NULL;
    if (!config_lookup_string(&cfg, "log_file", &log_file)) {
        printf("Failed to read \"log_file\" attribute from config '%s'", SERVER_CONFIG);
        config_destroy(&cfg);
        return 0;
    }
    
    int log_level = 0;
    if (!config_lookup_int(&cfg, "log_level", &log_level)) {
        printf("Failed to read \"log_level\" attribute from config '%s'", SERVER_CONFIG);
        config_destroy(&cfg);
        return 0;
    }

    config_context.maildir = malloc(strlen(maildir) * sizeof(char));
    strcpy(config_context.maildir, maildir);

    config_context.domain = malloc(strlen(domain) * sizeof(char));
    strcpy(config_context.domain, domain);

    config_context.log_file = malloc(strlen(log_file) * sizeof(char));
    strcpy(config_context.log_file, log_file);
        
    config_context.port = port;
    config_context.log_level = log_level;

    config_destroy(&cfg);
    return 1;
}

void free_config() {
    free(config_context.domain);
    free(config_context.maildir);
    free(config_context.log_file);
    memset(&config_context, 0, sizeof(config));
}
