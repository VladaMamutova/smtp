#include <stdio.h>
#include "smtp.h"

#define DIRECTORY_OTHER_SERVERS ".OTHER_SERVERS"

typedef struct maildir_other_server {
    char *directory;
    char *server_name;

    char **message_full_file_names;
    int messages_count;

    int is_connected;

    smtp_context* smtp_context;
    smtp_response* response;
} maildir_other_server;


typedef struct maildir_main {
    char *directory; //дирретория
    maildir_other_server *servers; //директории с уделенными серверами
    int servers_size;
} maildir_main;

maildir_main *init_maildir(char *directory);
int read_maildir_servers(maildir_main *maildir);
void read_maildir_servers_new(maildir_other_server *server);
int find_server_num_by_name(char *name, maildir_main *maildir);