#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 6969
#define BUFFER_SIZE 1024

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg); // Free the dynamically allocated memory

    char buffer[BUFFER_SIZE];
    // Print a message to indicate that the thread is handling a request
    printf("Thread %ld: Handling request from client socket %d\n", pthread_self(), client_socket);

    // Read client request
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    // Send HTTP response
    const char *response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n\r\n"
                           "<h1>Hello, World!</h1>are.setmkaresnahoisetnarsioteanrsoit asoiaresntaoirsetaiorsetnaorisetn aorisetnaoirsutn aowfultn a9;o8lt aosryitn aroiset naoysutn asoitenas oytluahsot ashtoyaursht aorsietah soteanhs to";
    send(client_socket, response, strlen(response), 0);

    // Close connection
    close(client_socket);
    return NULL;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        return 1;
    }

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    // Listen for incoming connections
    listen(server_fd, 5);
    printf("HTTP server listening on port %d\n", PORT);

    // Accept and handle connections
    while ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        int *new_socket = malloc(sizeof(int));
        if (!new_socket) {
            perror("Memory allocation failed");
            close(client_socket);
            continue;
        }
        *new_socket = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)new_socket) != 0) {
            perror("Thread creation failed");
            free(new_socket);
            close(client_socket);
        } else {
            pthread_detach(thread_id); // Detach thread to avoid memory leaks
        }
    }

    close(server_fd);
    return 0;
}
