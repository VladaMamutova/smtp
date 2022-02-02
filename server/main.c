#include "config.h"
#include "log.h"
#include "server.h"

#include <stdio.h>
#include <sys/types.h> // pid_t
#include <unistd.h>  // fork

#include <stdlib.h> // exit

int main()
{
    if (!load_config()) {
        exit(1);
    }

    set_log_level(config_context.log_level);

    pid_t log_pid;
    switch(log_pid = fork()) {
    case -1:
        printf("Error: Log process fork() failed\n");
        exit(1); // выход из родительского процессса
    case 0:
        printf("Log process <%d> started!\n", getpid());
        start_logger(config_context.log_file);
    default:
        printf("Server process <%d> started!\n", getpid());
        run_server();
    }

    free_config();
    return 0;
}