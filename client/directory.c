#include "include/directory.h"
#include "include/util.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

maildir_main *init_maildir(char *directory)
{
    if (directory == NULL)
    {
        return NULL;
    }

    maildir_main *maildir = malloc(sizeof(maildir_main));
    memset(maildir, 0, sizeof(maildir_main));

    struct stat stat_info;
    if (!stat(directory, &stat_info))
    {
        if (S_ISDIR(stat_info.st_mode))
        {
            maildir->directory = malloc(strlen(directory) * sizeof(char));
            strcpy(maildir->directory, directory);
        }
    }

    return maildir;
}

// считываем дирректорию с письмами для удаленных сервреров
int read_maildir_servers(maildir_main *maildir)
{
    int newCount = 0;
    if (maildir->directory == NULL)
    {
        log_e("%s", "Error read  maildir: dir not found");
        return 0;
    }

    struct stat stat_info; //информация о файле
    if (!stat(maildir->directory, &stat_info))
    {
        if (!S_ISDIR(stat_info.st_mode))
        { // проверяем режим доступа
            log_e("Error read  maildir: %s", maildir->directory);
            return 0;
        }
    }

    //  получаем path_other_servers = "maildir/otherservers"
    char *path_other_servers = NULL;
    path_other_servers = malloc((strlen(maildir->directory) + 1 + strlen(DIRECTORY_OTHER_SERVERS)) * sizeof(char));
    strcpy(path_other_servers, "");
    strcat(path_other_servers, maildir->directory);
    strcat(path_other_servers, "/");
    strcat(path_other_servers, DIRECTORY_OTHER_SERVERS);

    int servers_count = 0;
    DIR *dir = opendir(path_other_servers);
    struct dirent *entry = readdir(dir);
    if (maildir->servers_size == 0)
    {
        maildir->servers = allocate_memory(sizeof(maildir_other_server));
    }
    else
    {
        servers_count = maildir->servers_size;
    }

    while (entry != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            int index = -1;
            for (int i = 0; i < maildir->servers_size; i++)
            {
                if (strcmp(maildir->servers[i].server_name, entry->d_name) == 0)
                {
                    index = i;
                    break;
                }
            }
            if (index < 0)
            {
                servers_count++;
                newCount++;
                maildir->servers = reallocate_memory(maildir->servers, sizeof(maildir_other_server) * (servers_count - 1),
                                                     sizeof(maildir_other_server) * servers_count);

                maildir_other_server server = {0};

                server.server_name = malloc(strlen(entry->d_name) * sizeof(char));
                strcpy(server.server_name, entry->d_name);

                server.directory = malloc((strlen(path_other_servers) + 1 + strlen(server.server_name)) * sizeof(char));
                strcpy(server.directory, "");
                strcat(server.directory, path_other_servers);
                strcat(server.directory, "/");
                strcat(server.directory, server.server_name);

                read_maildir_servers_new(&server);

                maildir->servers[servers_count - 1] = server;
            } else{
                read_maildir_servers_new(&maildir->servers[index]);
            }
        }
        entry = readdir(dir);
    }
    closedir(dir);

    maildir->servers_size = servers_count;

    log_d("maildir->servers_size = %d", maildir->servers_size);

    free(path_other_servers);

    return newCount;
}

//считывание новых писем из дирректории
void read_maildir_servers_new(maildir_other_server *server)
{
    if (server == NULL)
    {
        log_e("%s", "error read: server = NULL");
        return;
    }
    if (server->directory == NULL)
    {
        log_e("%s", "Error read: server.directory == NULL");
        return;
    }

    struct stat stat_info;
    if (!stat(server->directory, &stat_info))
    {
        if (!S_ISDIR(stat_info.st_mode))
        {
            log_e("Error read maildir: %s", server->directory);
            return;
        }
    }

    DIR *dir = opendir(server->directory);
    struct dirent *entry = readdir(dir);

    int messages_count = 0;
    if (server->messages_count == 0)
    {
        server->message_full_file_names = allocate_memory(sizeof(char *));
    }
    else
    {
        messages_count = server->messages_count;
    }

    while (entry != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, "tmp") != 0)
        {
            int newMesg = 1;
            for (int i = 0; i < server->messages_count; i++)
            {
                if (strcmp(server->message_full_file_names[i], entry->d_name) == 0)
                {
                    newMesg = 0;
                    break;
                }
            }
            if (newMesg)
            {
                messages_count++;
                server->message_full_file_names = reallocate_memory(server->message_full_file_names,
                                                                    sizeof(char *) * (messages_count - 1),
                                                                    sizeof(char *) * messages_count);
                server->message_full_file_names[messages_count - 1] = malloc((1 + strlen(server->directory) + strlen(entry->d_name)) * sizeof(char));
                strcpy(server->message_full_file_names[messages_count - 1], "");
                strcat(server->message_full_file_names[messages_count - 1], server->directory);
                strcat(server->message_full_file_names[messages_count - 1], "/");
                strcat(server->message_full_file_names[messages_count - 1], entry->d_name);
            }
        }
        entry = readdir(dir);
    }
    closedir(dir);

    server->messages_count = messages_count;
}