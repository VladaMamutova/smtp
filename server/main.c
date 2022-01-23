#include "server.h"
#include "log.h"

#include <stdio.h>
#include <sys/types.h> // pid_t
#include <unistd.h>  // fork

#include <stdlib.h> // exit

log_level cur_level;

int main()
{
    char *log_file = "logs/log.txt";

    pid_t log_pid;
    switch(log_pid = fork()) {
    case -1:
        printf("Error: Log process fork() failed\n");
        exit(1); // выход из родительского процессса
    case 0:
        printf("Log process <%d> started!\n", getpid());
        cur_level = DEBUG;
        start_logger(log_file);
    default:
        printf("Server process <%d> started!\n", getpid());
        run_server();
    }

    return 0;
}