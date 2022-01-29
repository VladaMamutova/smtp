#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include "include/directory.h"
#include "include/util.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>

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

                maildir->servers[servers_count - 1] = server;
            }
        }
        entry = readdir(dir);
    }
    closedir(dir);

    maildir->servers_size = servers_count;

    log_i("maildir->servers_size = %d", maildir->servers_size);

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
            log_e("Error read  maildir: %s", server->directory);
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
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, "error") != 0)
        {
            int newMesg = 1;
            for (int i = 0; i < server->messages_count; i++)
            {
                char* msg = malloc((1 + strlen(server->directory) + strlen(entry->d_name)) * sizeof(char));
                strcpy(msg, "");
                strcat(msg, server->directory);
                strcat(msg, "/");
                strcat(msg, entry->d_name);
                if (strcmp(server->message_full_file_names[i], msg) == 0)
                {
                    newMesg = 0;
                    break;
                }
                free(msg);
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

message *get_message(maildir_other_server *server)
{

    message *msg = NULL;
    time_t min_time;
    time(&min_time);
    int min;
    if (server != NULL && server->messages_count)
    {
        for (int i = 0; i < server->messages_count; i++)
        {
            struct stat stat_info;
            if (!stat(server->directory, &stat_info))
            {
                struct tm *timeinfo = localtime(&stat_info.st_ctime);
                time_t select_time = mktime(timeinfo);
                if (select_time < min_time)
                {
                    min_time = select_time;
                    min = 0;
                }
            }
        }
        msg = parse_message(server->message_full_file_names[min]);
    }
    return msg;
}

message *parse_message(char *filepath)
{
    FILE *fp;
    if ((fp = fopen(filepath, "r")) != NULL)
    {
        message *mes = allocate_memory(sizeof(message));
        memset(mes, 0, sizeof(message));

        char *string = NULL;
        int strings_count = 0;
        mes->line = allocate_memory(sizeof(char *));

        while ((string = file_readline(fp)) != NULL)
        {
            trim(string);
            my_pair *p;
            if (strcmp(string, "\n") != 0 && strcmp(string, "\r\n") != 0)
            {
                p = get_header(string);
            } else {
                free(string);
                break;
            }

            if (p != NULL)
            {
                if (strcmp(p->first, "from") == 0)
                {
                    asprintf(&mes->from, "%s", p->second);
                    free(p->first);
                    free(p->second);
                    free(p);
                    strings_count++;
                }
                else if (strcmp(p->first, "to") == 0)
                {
                    string_tokens tokens = split(p->second, ",");

                    mes->to_size = tokens.count_tokens;
                    mes->to = callocate_memory(mes->to_size, sizeof(char *));

                    for (size_t i = 0; i < mes->to_size; i++)
                    {
                        asprintf(&mes->to[i], "%s", tokens.tokens[i].chars);
                    }
                    strings_count++;
                    free_string_tokens(&tokens);
                    free(p->first);
                    free(p->second);
                    free(p);
                } else  if (strcmp(p->first, "subject") == 0)
                {                   
                    free(p->first);
                    free(p->second);
                    free(p);
                    strings_count++;
                }
                if (mes->line != NULL )
                {
                    if(strings_count-1 > 0){                           
                        mes->line = reallocate_memory(mes->line, sizeof(char *) * (strings_count -1),
                                                sizeof(char *) * (strings_count ));
                    }
                }
                asprintf(&mes->line[strings_count-1], "%s", string);
            }
            free(string);
        }
        if(strings_count < 3){
            log_i("Error headers not found: %s", filepath);
            move_msg_to_error(filepath);
            return NULL;
        }

        while ((string = file_readline(fp)) != NULL)
        {
            trim(string);
           
            mes->line = reallocate_memory(mes->line, sizeof(char *) * strings_count,
                                              sizeof(char *) * (strings_count + 1));
            
            asprintf(&mes->line[strings_count], "%s", string);
            strings_count++;
            free(string);
        }

        if (feof(fp))
        {
            mes->line_size = strings_count;
            asprintf(&mes->directory, "%s", filepath);
        }
        else
        {
            log_i("Error when reading file: %s", filepath);
        }

        fclose(fp);
        return mes;
    }

    return NULL;
}

my_pair *get_header(char *line)
{
    if (line == NULL)
    {
        log_i("%s", "Error read message header");
        return NULL;
    }

    my_pair *p = allocate_memory(sizeof(my_pair));

    string_tokens tokens = split(line, ":");

    if (tokens.tokens != NULL)
    {
        char *str = tokens.tokens[0].chars;

        if (strstr(str, "from") != NULL)
        {
            asprintf(&p->first, "%s", "from");
        }
        else if (strstr(str, "to") != NULL)
        {
            asprintf(&p->first, "%s", "to");
        }else if (strstr(str, "subject") != NULL)
        {
            asprintf(&p->first, "%s", "subject");
        }

        if (tokens.count_tokens == 2)
        {
            str = tokens.tokens[1].chars;
            trim(str);
            if (str[strlen(str) - 1] == '\n' && str[strlen(str) - 2] == '\r')
            {
                str[strlen(str) - 1] = 0;
                str[strlen(str) - 1] = 0;
            }
            if (str[strlen(str) - 1] == '\n')
            {
                str[strlen(str) - 1] = 0;
            }
            asprintf(&p->second, "%s", tokens.tokens[1].chars);
        } else{
            return NULL;
        }
        free_string_tokens(&tokens);
        return p;
    }

    return NULL;
}

void delete_msg(maildir_other_server *server, message *msg, int flg)
{
    if (msg == NULL || msg->directory == NULL)
    {
        log_i("%s", "Unable remove message: msg = NULL || msg->directory == NULL");
        return;
    }
    if (flg == 1)
    {
        move_msg_to_error(msg->directory);
    }

    struct stat stat_info;
    if (!stat(msg->directory, &stat_info))
    {
        if (!S_ISREG(stat_info.st_mode))
        {
            log_i("%s", "Unuble remove message: - not file");
            // log_i("Unuble remove message: %s - not file", msg->directory);

            return;
        }
    }

    if (strstr(msg->directory, DIRECTORY_OTHER_SERVERS) != NULL)
    {
        if (remove(msg->directory) != 0)
        {
            log_i("%s", "Fail when remove message");
            // log_i("Fail when remove message %s", msg->directory);

            return;
        }

        if (server->message_full_file_names != NULL)
        {
            for (int i = 0; i < server->messages_count; i++)
            {
                if (strcmp(server->message_full_file_names[i], msg->directory) == 0)
                {
                    free(server->message_full_file_names[i]);
                    server->message_full_file_names[i] = NULL;
                    for (int j = i; j < server->messages_count - 1; j++)
                    {
                        server->message_full_file_names[j] = server->message_full_file_names[j + 1];
                    }
                    server->messages_count--;
                    break;
                }
            }
        }
    }

    free_message(msg);
}

void move_msg_to_error(char *oldPath){

        char *pch = strchr(oldPath, '/'); 
        int count = 0;   
        while (pch != NULL) {
            pch = strchr(pch+1, '/');
            count++;
        }
        pch = strchr(oldPath, '/'); 
        for(int i =0; i<count-1; i++){
            pch = strchr(pch+1, '/');
        }
        char* mailname = malloc(sizeof(char) * (strlen(oldPath)));
        strcpy(mailname, pch);       

        char *newPath = malloc(sizeof(char) * (strlen(oldPath) + 7));
        
        int lenOld = strlen(oldPath) ;
        int lenMail = strlen(mailname);
        int lenNew = (strlen(oldPath) - strlen(mailname));
        printf("lenOld = %d, lenMail = %d,  lenNew = %d\n",  lenOld, lenMail, lenNew);

        strcat(newPath,"");
        strncat(newPath, oldPath, (strlen(oldPath) - strlen(mailname)) );
        strcat(newPath, "/error");        
        strcat(newPath, mailname);
        rename(oldPath, newPath);
        
}

void free_message(message *msg)
{
    free(msg->directory);
    for (int i = 0; i < msg->to_size; i++)
    {
        free(msg->to[i]);
    }
    free(msg->to);
    free(msg->from);
    for (int i = 0; i < msg->line_size; i++)
    {
        free(msg->line[i]);
    }
    free(msg->line);
    free(msg);
}
