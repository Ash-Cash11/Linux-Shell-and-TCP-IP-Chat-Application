/*
 * Multi-Client TCP Chat Server
 * Linux/POSIX Implementation
 * Uses: socket(), bind(), listen(), accept(), select(), recv(), send()
 * Compile: gcc server.c -o server
 * Run:     ./server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>          // close(), read(), write()
#include <sys/socket.h>      // socket(), bind(), listen(), accept(), send(), recv()
#include <sys/select.h>      // select(), FD_SET(), FD_ZERO(), FD_ISSET()
#include <netinet/in.h>      // struct sockaddr_in, htons(), INADDR_ANY
#include <arpa/inet.h>       // inet_ntoa() for printing client IP

#define PORT        8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main()
{
    int server_sock;                      // server socket descriptor
    int client_sock[MAX_CLIENTS];         // array to hold client socket descriptors
    int new_sock;                         // newly accepted client socket
    struct sockaddr_in address;           // server address struct
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    fd_set readfds;                       // file descriptor set for select()

    // Initialize all client sockets to 0 (empty)
    for (int i = 0; i < MAX_CLIENTS; i++)
        client_sock[i] = 0;

    // -------------------------------------------------------
    // STEP 1: Create the server socket (TCP = SOCK_STREAM)
    // -------------------------------------------------------
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // Allow port reuse to avoid "Address already in use" error on restart
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt() failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // -------------------------------------------------------
    // STEP 2: Bind socket to IP and PORT
    // -------------------------------------------------------
    address.sin_family      = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY;      // bind to all available interfaces
    address.sin_port        = htons(PORT);     // host-to-network byte order

    if (bind(server_sock, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("bind() failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // -------------------------------------------------------
    // STEP 3: Listen for incoming connections
    // -------------------------------------------------------
    if (listen(server_sock, MAX_CLIENTS) == -1)
    {
        perror("listen() failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // -------------------------------------------------------
    // STEP 4: Main loop — use select() to monitor all sockets
    // -------------------------------------------------------
    while (1)
    {
        // Clear and rebuild the file descriptor set every iteration
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);          // always watch the server socket
        int max_fd = server_sock;               // track the highest fd for select()

        // Add all active client sockets to the set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_sock[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_fd)
                max_fd = sd;
        }

        // select() blocks here until activity is detected on any socket
        // max_fd + 1 tells select() the range of fds to monitor
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity == -1)
        {
            perror("select() failed");
            continue;
        }

        // -------------------------------------------------------
        // STEP 5: New incoming connection on server socket
        // -------------------------------------------------------
        if (FD_ISSET(server_sock, &readfds))
        {
            new_sock = accept(server_sock, (struct sockaddr *)&address,
                              (socklen_t *)&addrlen);
            if (new_sock == -1)
            {
                perror("accept() failed");
                continue;
            }

            printf("New client connected — IP: %s, Port: %d\n",
                   inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new socket to the first empty slot in client array
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_sock[i] == 0)
                {
                    client_sock[i] = new_sock;
                    printf("Added client to slot %d\n", i);
                    break;
                }
            }
        }

        // -------------------------------------------------------
        // STEP 6: Handle message from an existing client
        // -------------------------------------------------------
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_sock[i];

            if (FD_ISSET(sd, &readfds))
            {
                int bytes_received = recv(sd, buffer, BUFFER_SIZE, 0);

                if (bytes_received <= 0)
                {
                    // Client disconnected
                    if (bytes_received == 0)
                        printf("Client in slot %d disconnected.\n", i);
                    else
                        perror("recv() failed");

                    close(sd);               // POSIX: close() instead of closesocket()
                    client_sock[i] = 0;      // free the slot
                }
                else
                {
                    // Null-terminate and print received message
                    buffer[bytes_received] = '\0';
                    printf("Client [slot %d]: %s\n", i, buffer);

                    // Broadcast message to all OTHER connected clients
                    for (int j = 0; j < MAX_CLIENTS; j++)
                    {
                        if (client_sock[j] != 0 && client_sock[j] != sd)
                        {
                            if (send(client_sock[j], buffer, bytes_received, 0) == -1)
                                perror("send() failed");
                        }
                    }
                }
            }
        }
    }

    // Clean up (reached only on forced exit)
    close(server_sock);
    return 0;
}
