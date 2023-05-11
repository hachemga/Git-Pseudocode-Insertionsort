#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define BACKLOG 3
#define MSG_SIZE 200
#define ARRAY_SIZE(x) ( sizeof(x) / sizeof((x)[0]) )

#define reply_400 "HTTP/1.1 400\r\nContent-Length: 0\r\n\r\n"
#define reply_404 "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n"
#define reply_501 "HTTP/1.1 501\r\nContent-Length: 0\r\n\r\n"
#define reply_201 "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n"
#define reply_200 "HTTP/1.1 200 Ok\r\nContent-Length: 0\r\n\r\n"
#define reply_204 "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n"
#define reply_403 "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n"

// The beginning of the code (from line 29 till 47) is inspired by the presentation "Sockets" (page 24) from the course material.
int main(int argc, char **argv) {
    // Start here :)
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 0;
    }

    // Get the port number.
    int port = atoi(argv[1]);
    int socketFD, new_fd;
    struct sockaddr_in s_addr;

    // socket()
    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit(1);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind()
    if (bind(socketFD, (struct sockaddr *) &s_addr, sizeof(s_addr)) < 0)
        exit(1);

    // listen()
    if (listen(socketFD, BACKLOG) != 0)
        exit(1);

    char msg[MSG_SIZE];
    int bytes_sent;

    char receive_buffer[1];
    char all_buffer[1024];
    char separator[] = "\r\n\r\n";
    int buffer_length = 0;

    char path[100][100];
    char payload[100][100];
    for (int i = 0; i < 100; i++) {
        memset(path[i], 0, 100);
        memset(payload[i], 0, 100);
    }

    strcpy(path[0], "/static/foo");
    strcpy(path[1], "/static/bar");
    strcpy(path[2], "/static/baz");

    strcpy(payload[0], "Foo");
    strcpy(payload[1], "Bar");
    strcpy(payload[2], "Baz");

    while (1) {
        // accept()
        if ((new_fd = accept(socketFD, (struct sockaddr *) NULL, NULL)) == -1)
            exit(1);

        // recv() and send()
        while (1) {
            // Receiving only one byte.
            int bytes_received = recv(new_fd, receive_buffer, 1, 0);
            if (bytes_received == -1) {
                exit(1);
            }
            if (bytes_received == 0) {
                break;
            }

            // Adding received byte to the buffer.
            all_buffer[buffer_length] = receive_buffer[0];
            buffer_length += bytes_received;
            all_buffer[buffer_length] = '\0';

            // If the buffer ends with "\r\n\r\n", there's a request and the server should reply.
            if (buffer_length > 4 && all_buffer[buffer_length - 4] == '\r' && all_buffer[buffer_length - 3] == '\n' &&
                all_buffer[buffer_length - 2] == '\r' && all_buffer[buffer_length - 1] == '\n') {

                // Get the first line from the request.
                char request_line[1024];
                memset(request_line, 0, 1024);
                for (int i = 0; i < buffer_length - 1; i++) {
                    if (all_buffer[i] == '\r' && all_buffer[i + 1] == '\n') {
                        strncpy(request_line, all_buffer, i);
                        request_line[i] = '\0';
                        break;
                    }
                }

                // Split request line by whitespace.
                char *ptr = strtok(request_line, " ");

                // True if GET/PUT/DELETE request, false otherwise.
                int get_flag = 0;
                int put_flag = 0;
                int delete_flag = 0;
                int content_length = 0;

                // Check the method.
                if (ptr == NULL)
                    strcpy(msg, reply_400);
                else if (strcmp(ptr, "GET") == 0) {
                    strcpy(msg, reply_404);
                    get_flag = 1;
                } else if ((strcmp(ptr, "PUT") == 0 || strcmp(ptr, "POST") == 0) &&
                           strstr(all_buffer, "Content-Length") == NULL)
                    strcpy(msg, reply_400);
                else if (strcmp(ptr, "PUT") == 0 && strstr(all_buffer, "Content-Length:") != NULL) {
                    put_flag = 1;
                    char *put_pointer = strstr(all_buffer, "Content-Length: ") + strlen("Content-Length: ");
                    content_length = atoi(put_pointer);

                } else if (strcmp(ptr, "DELETE") == 0) {
                    if (strstr(all_buffer, "Content-Length: ") != NULL) {
                        char *put_pointer = strstr(all_buffer, "Content-Length: ") + strlen("Content-Length: ");
                        content_length = atoi(put_pointer);
                    }
                    delete_flag = 1;
                } else
                    strcpy(msg, reply_501);

                // URL check.
                ptr = strtok(NULL, " ");

                // if ? ptr[0] != '/'
                if (ptr == NULL)
                    strcpy(msg, reply_400);
                else if (get_flag) {
                    for (int i = 0; i < ARRAY_SIZE(path); i++) {
                        if (strcmp(ptr, path[i]) == 0) {
                            snprintf(msg, MSG_SIZE - 1,
                                     "HTTP/1.1 200\r\nContent-Length: %lu\r\nContent-Type: text/html\r\nAccept-Ranges: bytes"
                                     "\r\n\r\n%s", strlen(payload[i]), payload[i]);
                            break;
                        } else {
                            strcpy(msg, reply_404);
                        }

                    }
                } else if (put_flag) {
                    // Receive payload.
                    char payload_buffer[content_length + 1];
                    if (recv(new_fd, payload_buffer, content_length, 0) <= 0) {
                        perror("recv()");
                        break;
                    }
                    payload_buffer[content_length] = '\0';

                    // The URI starts with "/dynamic/".
                    if (strncmp(ptr, "/dynamic/", strlen("/dynamic/")) == 0) {
                        int index = -1;
                        // Find the index of the given path.
                        for (int i = 0; i < ARRAY_SIZE(path); i++) {
                            if (strcmp(ptr, path[i]) == 0) {
                                index = i;
                                strcpy(msg, reply_200);
                                break;
                            }
                        }

                        // If given path is not in the array, fins the first empty slot.
                        if (index == -1) {
                            for (int i = 0; i < ARRAY_SIZE(path); i++) {
                                if (strlen(path[i]) == 0) {
                                    index = i;
                                    strcpy(msg, reply_201);
                                    break;
                                }
                            }
                        }

                        strncpy(path[index], ptr, strlen(ptr));
                        strncpy(payload[index], payload_buffer, content_length);
                    } else {
                        strcpy(msg, reply_403);
                    }
                } else if (delete_flag) {
                    // The URI starts with "/dynamic/".
                    if (strncmp(ptr, "/dynamic/", strlen("/dynamic/")) == 0) {
                        //Receive payload.
                        char payload_buffer[content_length + 1];
                        if (recv(new_fd, payload_buffer, content_length, 0) <= 0) {
                            perror("recv()");
                            break;
                        }
                        payload_buffer[content_length] = '\0';
                        int index = -1;
                        // Find the index of the given path.
                        for (int i = 0; i < ARRAY_SIZE(path); i++) {
                            if (strcmp(ptr, path[i]) == 0) {
                                index = i;
                                strcpy(msg, reply_200);
                                memset(path[index], 0, 100);
                                memset(payload[index], 0, 100);
                                break;
                            }
                        }
                        // If given path is not in the array, fins the first empty slot.
                        if (index == -1) {
                            strcpy(msg, reply_404);
                        }
                    } else {
                        strcpy(msg, reply_403);
                    }
                }

                // HTTPVersion check.
                ptr = strtok(NULL, " ");
                if (ptr == NULL || strstr(ptr, "HTTP/") == NULL)
                    strcpy(msg, reply_400);

                if (strtok(NULL, " ") != NULL)
                    strcpy(msg, reply_400);

                bytes_sent = send(new_fd, msg, strlen(msg), 0);

                // Removing request from the buffer.
                memset(all_buffer, 0, 1024);
                buffer_length = 0;

                if (bytes_sent == -1) {
                    perror("send()");
                    exit(1);
                }
            }
        }
    }
    return 0;
}
