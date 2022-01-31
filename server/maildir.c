#include "maildir.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

char *maildir_base;

int init_maildir()
{
    ensure_dir(config_context.maildir);

    int maildir_base_size = strlen(config_context.maildir) + strlen(DIRECTORY_OTHER_SERVERS) + 2;
    maildir_base = malloc(maildir_base_size);
    sprintf(maildir_base, "%s/%s", config_context.maildir, DIRECTORY_OTHER_SERVERS);

    return ensure_dir(maildir_base) == 0;
}

char* get_maildir_filename(const char *domain)
{
    int dirname_size = strlen(maildir_base) + 1 + strlen(domain);
    char* dirname = malloc(dirname_size);
    sprintf(dirname, "%s/%s", maildir_base, domain);

    ensure_dir(dirname);

    int filename_size = dirname_size + 1 + FILENAME_SIZE * sizeof(char) + 1;
    char* filename = malloc(filename_size);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *time;
    time = localtime(&tv.tv_sec);
    char time_info[64];
    strftime (time_info, sizeof(time_info), "%d%m%Y%H%M%S", time);

    sprintf(filename, "%s/%s%ld", dirname, time_info, tv.tv_usec);

    return filename;
}

int ensure_dir(const char* dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        if (mkdir(dir, 0755) == -1) {
            return -1;
        }
    }
    return 0;
}
