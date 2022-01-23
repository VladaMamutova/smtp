#include <libconfig.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#define APPLICATION_CONFIG "/home/varvara/tcp-ip/smtp/client/application.cfg"

typedef struct config {
    char * maildir;
    int debug;
    int logs_on;
    char *hostname;
    char *server_port;
} config;

extern config config_context;
bool loading_config();
int init_signals_handler();
void destroy_configuration();