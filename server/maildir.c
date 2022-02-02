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

char* get_tmp_maildir_filename(const char *username, const char *domain)
{
    char *base_dirname = get_base_dir(username, domain);
    int base_dirname_size = strlen(base_dirname);
    ensure_dir(base_dirname);

    const char* tmpdir = "tmp";
    int dirname_size = base_dirname_size + 1 + strlen(tmpdir);
    char *dirname = malloc(dirname_size);
    sprintf(dirname, "%s/%s", base_dirname, tmpdir);
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

    free(base_dirname);
    free(dirname);

    return filename;
}

char* get_new_maildir_filename(const char *username, const char *domain)
{
    char *base_dirname = get_base_dir(username, domain);
    int base_dirname_size = strlen(base_dirname);
    ensure_dir(base_dirname);

    if (strcmp(domain, config_context.domain) == 0) {
        const char* newdir = "new";
        int dirname_size = base_dirname_size + 1 + strlen(newdir);
        char *dirname = malloc(dirname_size);
        sprintf(dirname, "%s/%s", base_dirname, newdir);
        ensure_dir(dirname);

        free(base_dirname);
        base_dirname = dirname;
        base_dirname_size = dirname_size;
    }

    int filename_size = base_dirname_size + 1 + FILENAME_SIZE * sizeof(char) + 1;
    char* filename = malloc(filename_size);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *time;
    time = localtime(&tv.tv_sec);
    char time_info[64];
    strftime (time_info, sizeof(time_info), "%d%m%Y%H%M%S", time);
    sprintf(filename, "%s/%s%ld", base_dirname, time_info, tv.tv_usec);

    free(base_dirname);

    return filename;
}

char* get_base_dir(const char *username, const char *domain)
{
    char *base_dirname;
    int base_dirname_size;
    if (strcmp(domain, config_context.domain) == 0) {
        base_dirname_size = strlen(config_context.maildir) + 1 + strlen(username) + 1;
        base_dirname = malloc(base_dirname_size);
        sprintf(base_dirname, "%s/%s", config_context.maildir, username);
    } else {
        base_dirname_size = strlen(maildir_base) + 1 + strlen(domain);
        base_dirname = malloc(base_dirname_size);
        sprintf(base_dirname, "%s/%s", maildir_base, domain);
    }

    return base_dirname;
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
