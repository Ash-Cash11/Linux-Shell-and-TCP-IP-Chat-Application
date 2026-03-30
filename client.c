#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, PORT);
    printf("Type your message and press Enter:\n");

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        if (select(sock + 1, &readfds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
            send(sock, buffer, strlen(buffer), 0);
        }

        if (FD_ISSET(sock, &readfds)) {
            int bytes = recv(sock, buffer, BUFFER_SIZE, 0);
            if (bytes <= 0) {
                printf("Server disconnected.\n");
                break;
            }
            buffer[bytes] = '\0';
            printf(">> %s", buffer);
        }
    }

    close(sock);
    return 0;
}
