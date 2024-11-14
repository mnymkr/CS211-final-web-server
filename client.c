#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8386
#define BUFFER_SIZE 100

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char request[BUFFER_SIZE], response[BUFFER_SIZE];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
//    if (sock < 0) {
//        perror("Socket creation failed");
//        return 1;
//    }

    // Set up server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // Create HTTP GET request
    sprintf(request, "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");

    // Send request to the server
    if (send(sock, request, strlen(request), 0) < 0) {
        perror("Send failed");
        close(sock);
        return 1;
    }

    // Receive response from the server
    int bytes_received = recv(sock, response, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
        close(sock);
        return 1;
    }

    // Null-terminate and print the response
    response[bytes_received] = '\0';
    printf("Server response:\n%s\n", response);

    // Close the socket
    close(sock);
    return 0;
}
