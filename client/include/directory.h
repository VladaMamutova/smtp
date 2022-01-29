#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdio.h>
#include "smtp.h"
#include "util.h"

#define DIRECTORY_NEW_MESSAGES "new"
#define DIRECTORY_CUR_MESSAGES "cur"
#define DIRECTORY_OTHER_SERVERS ".OTHER_SERVERS"


typedef struct message {
    char *directory;

    char *from;
    char **to;
    size_t to_size;

    char **line;
    size_t line_size;

    char* date;
} message;

typedef struct maildir_other_server {
    char *directory;
    char *server_name;

    char **message_full_file_names;
    int messages_count;
    message* cur_msg;

    int is_connected;

    smtp_context* smtp_context;
    smtp_response* response;

    int iteration;
    int error;
} maildir_other_server;


typedef struct maildir_main {
    char *directory; //дирретория
    maildir_other_server *servers; //директории с уделенными серверами
    int servers_size;
} maildir_main;

maildir_main *init_maildir(char *directory);

int read_maildir_servers(maildir_main *maildir);
void read_maildir_servers_new(maildir_other_server *server);
message *get_message(maildir_other_server *server);
message *parse_message(char *filepath);
my_pair *get_header(char *line); 
int find_server_num_by_name(char *name, maildir_main *maildir);
void delete_msg(maildir_other_server *server, message *mes, int flg);
void move_msg_to_error(char *oldPath);
void free_message(message *mess);
#endif