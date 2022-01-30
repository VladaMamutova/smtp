#include "log.h"
#include "queue.h"

#include <fcntl.h>
#include <signal.h> // signal

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // access

#define PROJECT_ID 16
#define FD_QUEUE_PATH "logs"

int queue_id;
key_t key; // ключ для системных вызовов
FILE *file_log = NULL;
log_level current_level;

int start_logger(const char *filename)
{
    printf("Logs will be saved to a file '%s'.\n", filename);

    signal(SIGINT, &handle_log_signal); // обработчик CTRL+C

    if ((file_log = fopen(filename, "w+")) == NULL) {
        printf("Error: fopen() failed");
        return stop_logger();
    }

    if ((access(FD_QUEUE_PATH, F_OK)) < 0) {
        printf("Error: access() failed");
        return stop_logger();
    }

    setvbuf(file_log, NULL, _IONBF, 0);

    if ((key = ftok(FD_QUEUE_PATH, PROJECT_ID)) < 0) {
        printf("Error: ftok() failed");
        return stop_logger();
    }
    if ((queue_id = msgget(key, 0644 | IPC_CREAT)) < 0) { // получаем идентификатор очереди для логов
        printf("Error: msgget() failed");
        return stop_logger();
    }

    ssize_t message_size;
    queue_msg_t queue_msg;

    while (1)
    {
        if (msgrcv(queue_id, &queue_msg, sizeof(queue_msg), 1, MSG_NOERROR) < 0) {
            printf("Error: msgrcv() failed");
            return stop_logger();
        }

        message_size = strlen(queue_msg.mtext);
        if (save_message(queue_msg.mtext, message_size) < 0) {
            printf("Error: save_message() failed");
            return stop_logger();
        }
    }

    return 0;
}

log_level get_log_level()
{
    return current_level;
}

void set_log_level(log_level log_level)
{
    current_level = log_level;
}

int log_message(log_level level, const char *message)
{
    if ((key = ftok(FD_QUEUE_PATH, PROJECT_ID)) < 0) {
        return -1;
    }
    if ((queue_id = msgget(key, 0644 | IPC_CREAT)) < 0) {
        return -1;
    }

    queue_msg_t queue_msg = {0};
    queue_msg.mtype = 1;
    strcpy(queue_msg.mtext, message);
    if (msgsnd(queue_id, &queue_msg, sizeof(queue_msg), IPC_NOWAIT) < 0) {
        return -1;
    }

    return 0;
}

int save_message(const char *message, ssize_t size)
{
    if (file_log == NULL || size < 0) {
        return -1;
    }
    if (size == 0) {
        return 0;
    }

    struct tm *now;
    char timestr[20];
    time_t timenow = time(NULL);
    now = localtime(&timenow);
    sprintf(timestr, "%d.%02d.%02d %02d:%02d:%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

    const int log_msg_size = 20 + 1 + size;
    char *log_msg = malloc(log_msg_size);
    strcpy(log_msg, timestr);
    strcat(log_msg, " ");
    strcat(log_msg, message);

    printf("%s", log_msg); // дублируем логи в консоль
    fwrite(log_msg, sizeof(char), log_msg_size, file_log);
    free(log_msg);
    return 0;
}

void handle_log_signal(int signal)
{
    printf("\nSignal <SIGINT> received. Stopping logger...\n");
    stop_logger();
}

int stop_logger()
{
    if (file_log != NULL) {
        fclose(file_log);
    }
    
    printf("Log process <%d> stopped.\n", getpid());
    exit(0);
}
