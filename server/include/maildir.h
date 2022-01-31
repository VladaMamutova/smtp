#ifndef MAILDIR_H_
#define MAILDIR_H_

#define DIRECTORY_OTHER_SERVERS ".OTHER_SERVERS"
#define FILENAME_SIZE 255

int init_maildir();
char* get_maildir_filename(const char *domain);
int ensure_dir(const char *dir);

#endif // MAILDIR_H_
