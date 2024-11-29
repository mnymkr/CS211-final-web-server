//
//  main.c
//  Server_multi
//
//  Created by Lê Giang on 22/11/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Global variables
volatile int active_threads = 0; // Đếm số threads đang hoạt động
pthread_mutex_t thread_count_lock = PTHREAD_MUTEX_INITIALIZER; // Mutex bảo vệ biến toàn cục

volatile sig_atomic_t keep_running = 1; // Điều khiển tắt server

// Signal handler để dừng server
void handle_sigint(int sig) {
    keep_running = 0;
    printf("\nShutting down server...\n");
}

// Hàm xử lý client
void *handle_client(void *client_socket) {
    int socket = *(int *)client_socket; // Copy socket từ tham số
    free(client_socket); // Giải phóng bộ nhớ

    // Tăng số lượng thread
    pthread_mutex_lock(&thread_count_lock);
    active_threads++;
    printf("Thread created. Active threads: %d\n", active_threads);
    pthread_mutex_unlock(&thread_count_lock);

    char buffer[BUFFER_SIZE] = {0};
    while (1) {
        ssize_t bytes_read = read(socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("Failed to read from client");
            }
            close(socket);

            // Giảm số lượng thread
            pthread_mutex_lock(&thread_count_lock);
            active_threads--;
            printf("Thread finished. Active threads: %d\n", active_threads);
            pthread_mutex_unlock(&thread_count_lock);

            pthread_exit(NULL);
        }

        buffer[bytes_read] = '\0'; // Null-terminate
        printf("Request received: %s\n", buffer);

        // Giả lập xử lý mất 1 giây
        sleep(1);

        // Gửi phản hồi
        const char *response_body = "<h1>Hello, Client!</h1>";
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %ld\r\n"
                 "Connection: keep-alive\r\n\r\n%s",
                 strlen(response_body), response_body);

        send(socket, response, strlen(response), 0);
    }
}

int main(void) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Đăng ký signal để dừng server
    signal(SIGINT, handle_sigint);

    // Tạo socket server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set socket options");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Bind socket tới port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen cho các kết nối đến
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Chấp nhận kết nối và tạo thread cho từng client
    while (keep_running) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread_id;
        int *new_socket = malloc(sizeof(int)); // Tạo socket động để thread sử dụng
        *new_socket = client_socket;

        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_socket) != 0) {
            perror("Thread creation failed");
            free(new_socket);
            close(client_socket);
        } else {
            pthread_detach(thread_id); // Tách thread để tự giải phóng tài nguyên sau khi hoàn thành
        }
    }

    close(server_fd);
    return 0;
}
