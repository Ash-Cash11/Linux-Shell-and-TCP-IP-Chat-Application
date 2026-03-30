#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main()
{
    int server_sock, new_sock;
    int client_sock[MAX_CLIENTS];
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int opt = 1;

    for (int i = 0; i < MAX_CLIENTS; i++)
        client_sock[i] = 0;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, MAX_CLIENTS) == -1) {
        perror("listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        int max_fd = server_sock;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sock[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_fd) max_fd = sd;
        }

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(server_sock, &readfds)) {
            new_sock = accept(server_sock, (struct sockaddr *)&address, &addrlen);
            if (new_sock == -1) {
                perror("accept");
                continue;
            }
            printf("New client connected - IP: %s Port: %d\n",
                   inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sock[i] == 0) {
                    client_sock[i] = new_sock;
                    printf("Client added to slot %d\n", i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sock[i];
            if (!FD_ISSET(sd, &readfds)) continue;

            int bytes = recv(sd, buffer, BUFFER_SIZE, 0);
            if (bytes <= 0) {
                printf("Client slot %d disconnected.\n", i);
                close(sd);
                client_sock[i] = 0;
            } else {
                buffer[bytes] = '\0';
                printf("Client [%d]: %s\n", i, buffer);
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (client_sock[j] != 0 && client_sock[j] != sd)
                        send(client_sock[j], buffer, bytes, 0);
                }
            }
        }
    }

    close(server_sock);
    return 0;
}
