#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "include/client.h"
#include "include/config.h"
#include "../common/include/log.h"

int main()
{
    pid_t log_pid;
    char *log_file = "logs/log.txt";

    set_log_level(DEBUG);
    switch (log_pid = fork())
    {
        case 0:
            start_logger(log_file);
            break;
        case -1:
            fprintf(stderr, "Can't fork log process\n");
            break;
        default:
        {
            loading_config();
            run_client();
            break;
        }
    }

    return 0;
}