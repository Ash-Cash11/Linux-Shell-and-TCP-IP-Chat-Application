/*
 * TCP Chat Client
 * Linux/POSIX Implementation
 * Compile: gcc client.c -o client
 * Run:     ./client
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>          // close(), read(), write()
#include <sys/socket.h>      // socket(), connect(), send(), recv()
#include <sys/select.h>      // select(), FD_SET(), FD_ZERO(), FD_ISSET()
#include <netinet/in.h>      // struct sockaddr_in, htons()
#include <arpa/inet.h>       // inet_pton() for IP address conversion

#define PORT        8080
#define SERVER_IP   "127.0.0.1"    // localhost — change for remote server
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    // -------------------------------------------------------
    // STEP 1: Create client socket
    // -------------------------------------------------------
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // -------------------------------------------------------
    // STEP 2: Set server address and connect
    // -------------------------------------------------------
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT);

    // Convert IP string to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton() failed — invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect() failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, PORT);
    printf("Type your message and press Enter:\n");

    // -------------------------------------------------------
    // STEP 3: Main loop — use select() to watch both
    //         stdin (keyboard) and the socket simultaneously
    // -------------------------------------------------------
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);   // watch keyboard input (fd = 0)
        FD_SET(sock, &readfds);           // watch incoming messages from server

        int max_fd = sock;                // sock > STDIN_FILENO (0) always

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity == -1)
        {
            perror("select() failed");
            break;
        }

        // -------------------------------------------------------
        // User typed a message — send it to server
        // -------------------------------------------------------
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
                break;   // EOF (Ctrl+D)

            if (send(sock, buffer, strlen(buffer), 0) == -1)
            {
                perror("send() failed");
                break;
            }
        }

        // -------------------------------------------------------
        // Message received from server — print it
        // -------------------------------------------------------
        if (FD_ISSET(sock, &readfds))
        {
            int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);

            if (bytes_received <= 0)
            {
                if (bytes_received == 0)
                    printf("Server disconnected.\n");
                else
                    perror("recv() failed");
                break;
            }

            buffer[bytes_received] = '\0';
            printf("Server: %s", buffer);
        }
    }

    close(sock);
    return 0;
}
