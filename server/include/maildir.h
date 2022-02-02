#ifndef MAILDIR_H_
#define MAILDIR_H_

#define DIRECTORY_OTHER_SERVERS ".OTHER_SERVERS"
#define FILENAME_SIZE 255

int init_maildir();
char* get_base_dir(const char *username, const char *domain);
char* get_tmp_maildir_filename(const char *username, const char *domain);
char* get_new_maildir_filename(const char *username, const char *domain);
int ensure_dir(const char *dir);

#endif // MAILDIR_H_
