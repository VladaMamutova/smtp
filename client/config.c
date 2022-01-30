#include "util.h"
#include <log.h>
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

config config_context = { 0 };

bool loading_config() {
    config_t cfg;
    config_init(&cfg);

    log_i("%s", "Load config");
    if (!(config_read_file(&cfg, APPLICATION_CONFIG))) {
        config_destroy(&cfg);
        return false;
    }

    const char *maildir_path = NULL;
    if (!config_lookup_string(&cfg, "maildir", &maildir_path)) {
        log_e("%s","Error read attr: maildir");
        return false;
    }
    int debug;
    if (!config_lookup_int(&cfg, "debug", &debug)) {
        log_e("%s","Error read attr: debug");
        return false;
    }

    if (debug != 0) {
        // log_i("%s", "Mode DEBUG is active");
    }

    const char *hostname = NULL;
    if (!config_lookup_string(&cfg, "hostname", &hostname)) {
        // log_e("%s","Error read attr: hostname");
        return false;
    }
    
    config_context.maildir = malloc(strlen(maildir_path) * sizeof(char));
    config_context.hostname = malloc(strlen(hostname) * sizeof(char));
    strcpy(config_context.hostname, hostname);
    strcpy(config_context.maildir, maildir_path);

    // log_i("mail_dir = %s", config_context.maildir);
    // log_i("log level = %d", debug);
    // log_i("hostname = %s", config_context.hostname);
   
    config_destroy(&cfg);
    return true;
}

void destroy_configuration() {
    free(config_context.hostname);
    free(config_context.server_port);
    free(config_context.maildir);
    memset(&config_context, 0, sizeof(config));
}