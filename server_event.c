#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h> // For time measurement

#define PORT 8080
#define MAX_CLIENTS 1000
#define BUFFER_SIZE 1024

void handle_client(int client_sock) {
    struct timespec start, end;
    //clock_gettime(CLOCK_MONOTONIC, &start); // Start time

    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Receive data from client
    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate received data
      //  printf("Received message: %s\n", buffer);

        // Send response
        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello World\n";
        send(client_sock, response, strlen(response), 0);
    }

    close(client_sock); // Close the client connection

    //clock_gettime(CLOCK_MONOTONIC, &end); // End time

    // Calculate elapsed time
    //double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0 +
                          (end.tv_nsec - start.tv_nsec) / 1000000.0;
    //printf("Client %d processed in %.2f ms\n", client_sock, elapsed_time);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set readfds;
    int client_sockets[MAX_CLIENTS];

    // Create server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Error listening on socket");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Set server socket to non-blocking mode
    fcntl(server_sock, F_SETFL, O_NONBLOCK);
    printf("Server is listening on port %d...\n", PORT);

    // Initialize client_sockets array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
    }

    //struct timespec server_start, server_end;
    //clock_gettime(CLOCK_MONOTONIC, &server_start); // Start measuring total server time

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);

        // Add active client sockets to readfds
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                FD_SET(client_sockets[i], &readfds);
            }
        }

        int activity = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Error in select");
            continue;
        }

        // Handle new connection
        if (FD_ISSET(server_sock, &readfds)) {
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
            if (client_sock < 0) {
                perror("Error accepting connection");
                continue;
            }

            //printf("New connection accepted: client_socket %d\n", client_sock);

            // Add client socket to client_sockets array
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == -1) {
                    client_sockets[i] = client_sock;
                    break;
                }
            }

            fcntl(client_sock, F_SETFL, O_NONBLOCK); // Set client socket to non-blocking mode
        }

        // Handle data from clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1 && FD_ISSET(client_sockets[i], &readfds)) {
                //printf("Handling client %d\n", client_sockets[i]);
                handle_client(client_sockets[i]);
                client_sockets[i] = -1; // Mark socket as free after handling
            }
        }
    }

    //clock_gettime(CLOCK_MONOTONIC, &server_end); // End measuring total server time

    //double total_elapsed = (server_end.tv_sec - server_start.tv_sec) * 1000.0 +
    //                       (server_end.tv_nsec - server_start.tv_nsec) / 1000000.0;
    //printf("Server ran for %.2f ms\n", total_elapsed);

    close(server_sock);
    return 0;
}


