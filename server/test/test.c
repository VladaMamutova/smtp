#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 2048
#define MAX_COMMAND_LENGTH 50
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int test_result(const char *expected, const char* actual)
{
    int result = expected == NULL && actual == NULL ||
                 expected != NULL && actual != NULL &&
                 strcmp(expected, actual) == 0;
    printf("TEST %s: %s %s %s \n",
        result ? "SUCCEEDED" : "FAILED", result ? "=" : "!=",
        expected, actual);

    return result;
}

char* get_mail(const char *message)
{
    char *start = strchr(message, '<') + 1;
    char *end = strchr(message, '>');
    char *at = strchr(message, '@');
    if (start == NULL || end == NULL || at == NULL) {
        return NULL;
    }

    int mail_size = end - start;
    if (mail_size < 3 || at - start < 1 || end - at - 1 < 1) {
        return NULL;
    }

    char *mail = malloc(sizeof(char) * (mail_size + 1));
    strncpy(mail, start, mail_size);
    mail[mail_size] = '\0';
    return mail;
}

int test_single_mail()
{
    // Arrange
    const char *command = "MAIL FROM: <vlada.mamutova@gmail.com>\r\n";

    // Act
    char *mail = get_mail(command);

    // Assert
    int result = test_result("vlada.mamutova@gmail.com", mail);
    if (mail != NULL) {
        free(mail);
    }

    return result;
}

int test_single_incorrect_mail()
{
    // Arrange
    const char *command = "MAIL FROM: <vlada.mamutova";

    // Act
    char *mail = get_mail(command);

    // Assert
    int result = test_result(NULL, mail);
    if (mail != NULL) {
        free(mail);
    }

    return result;
}

char *read_file(const char* filename)
{
    size_t file_size = 1024 * 1024 * 20;
    char *file_content = calloc(file_size, sizeof(char));
    if (!file_content) {
        printf("Failed to init buffer");
        return NULL;
    }

    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) {
        printf("Failed to open file '%s'", filename);
        return NULL;
    }

    char buffer[2048];
    int offset = 0;
	while(fgets(buffer, sizeof(buffer), file))
	{
        strcpy(file_content + offset, buffer);
        offset += strlen(buffer);
	}
    fclose(file);

    file_content[offset] = '\0';
    return file_content;
}

int send_command(int socket, const char* command, const char* expected_response) {
    int success;
    char buffer[BUFFER_SIZE + 1];
    int command_size = strlen(command);
    int offset = 0;
    int buffer_size;
    while (offset < command_size && success)
    {
        buffer_size = MIN(command_size - offset, BUFFER_SIZE);
        buffer[0] = '\0';
        strncpy(buffer, command + offset, buffer_size);
        buffer[buffer_size] = '\0';

        success = send(socket, buffer, buffer_size, 0);
        if (success <= 0) {
            if (command_size < MAX_COMMAND_LENGTH) {
                printf("Failed to send command \"%s\" (%d: %s)", command, errno, strerror(errno));
            } else {
                printf("Failed to send buffer (%d: %s):\n%s", errno, strerror(errno), buffer);
            }
            return 0;
        }
        printf("send buffer \n%s\n\n", buffer);
        offset += buffer_size;
    }

    if (success) {
        const char* command_end = command_size < MAX_COMMAND_LENGTH ? "\r\n" : "\r\n.\r\n";
        if (send(socket, command_end, strlen(command_end), 0) <= 0) {
            printf("Failed to send %s (%d: %s).",
                command_size < MAX_COMMAND_LENGTH ? command : "a big message",
                errno, strerror(errno));
        } else {
            printf("%s\n", command_size < MAX_COMMAND_LENGTH ? command : "<Sent a big message>");
        }
    }

    sleep(1);

    char response[256];
    success = recv(socket, response, sizeof(response), 0);
    if (strstr(response, expected_response) == NULL) {
        printf("Command \"%s\" failed with %s", command, response);
        return 0;
    }
    response[success] = '\0';
    printf("%s\n", response);

    return 1;
}

int main()
{
    //test_single_mail();
    //test_single_incorrect_mail();

    char *file_content = read_file("test.txt");
    if (file_content == NULL) {
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    server_addr.sin_port = htons(50025);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(client_socket, (struct sockaddr *) &server_addr,
        sizeof(server_addr)) < 0) {
        printf("Failed to connect to the server (127.0.0.1:50025)");
        exit(1);
    }

    char response[256];
    if (recv(client_socket, response, sizeof(response), 0) <= 0 ||
        strstr(response, "220") == NULL) {
        printf("Failed to connect to the server (127.0.0.1:50025)");
        exit(1);
    }
    printf("Connected to the server 127.0.0.1:50025\nStart testing...\n\n");
    
    char start_mail_commands[4][MAX_COMMAND_LENGTH] = {
        "HELO",
        "MAIL FROM: <mytestemail@vvserver.com>",
        "RCPT TO: <receiver1.email@vvserver.com>",
        "RCPT TO: <receiver2.email@gmail.com>"
    };

    const char* data_command = "DATA";
    const char* quit_command = "QUIT";

    int success = 1;
    for (size_t i = 0; i < 4 && success; i++) {
        success = send_command(client_socket, start_mail_commands[i], "250");
    }

    if (success) {
        success = send_command(client_socket, data_command, "354");
        if (success) {
            success = send_command(client_socket, file_content, "250");
        }
    }

    printf("%s\n\n", success ? "TESTS PASSED!" : "TESTS FAILED!");

    send_command(client_socket, quit_command, "221");
    
    free(file_content);
    close(client_socket);

    return 0;
}