// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 300 /* Maximum length of pending connections */

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // Read filename from client socket and store it into the buffer
    int bytes_received = read(client_socket, filename, sizeof(filename) - 1);
    filename[bytes_received] = '\0'; // Null-terminate the string
    // printf("Requested filename: %s\n", filename);

    // Open the requested file
    FILE *fp = fopen(filename, "r"); /* file pointer points to the first position */
    if (fp == NULL) {
        const char *error_msg = "File not found\n";
        send(client_socket, error_msg, strlen(error_msg), 0);
        close(client_socket);
        return NULL;
    }

    // Read file content and send it to the client
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        send(client_socket, buffer, strlen(buffer), 0);
    }
    fclose(fp);
    close(client_socket);
    return NULL;
}

int main() {
    int server_socket; // file descriptor for server listening socket

    // Set up the server address structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;   /* IPv4 address */
    server_addr.sin_addr.s_addr = INADDR_ANY; /* Receive all requests to any hosts' IP addresses + port */
    server_addr.sin_port = htons(PORT); /* convert to network byte order */
    /*
    Describes an IPv4 Internet domain socket address
    struct sockaddr_in {
        sa_family_t     sin_family;     (AF_INET)
        in_port_t       sin_port;       (Port number)
        struct in_addr  sin_addr;       (IPv4 address)
    };
    struct in_addr {
        in_addr_t s_addr;
    };
    typedef uint32_t in_addr_t;
    */

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0); /* SOCK_STREAM: tcp_socket */
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set up options on socket
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET,
                   SO_REUSEPORT, &opt, /*SO_REUSEPORT: allow multiple threads listen on the same port*/
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(server_socket, SOL_SOCKET,
                   SO_REUSEADDR, &opt, /*SO_REUSEADDR: allow immediate restart of the server*/
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind the socket: assigning the given source IP address <address, port> to the socket  
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listen(server_socket, BACKLOG);
    printf("Server listening on port %d\n", PORT);

    int *new_socket;   // pointer to file descriptor for the new socket created when client connects to server
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    while (1) {
        new_socket = malloc(sizeof(int));
        // Extract first connection request on the pending queue, client_addr filled with peer address
        *new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addrlen);
        if (*new_socket < 0) {
            perror("Accept failed");
            free(new_socket);
            continue;
        }

        // Multi-thread
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, new_socket) != 0) {
            perror("Could not create thread");
            free(new_socket);
            close(*new_socket);
            continue;
        }
        
        pthread_detach(thread_id); // Detach thread to free resources when done
        
    }

    close(server_socket);
    return 0;
}