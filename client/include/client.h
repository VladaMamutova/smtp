#include <poll.h>

typedef struct context {
    struct pollfd * poll_fds;    
    int conn_context_count;
} context;

void run_client();
void handle_signal(int signal);