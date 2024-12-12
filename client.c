#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h> // For time measurement

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define TOTAL_REQUESTS 500 // Tổng số requests

void *send_request(void *arg) {
    struct sockaddr_in server_addr;
    char *request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    char buffer[BUFFER_SIZE];

    // Cấu hình địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Tạo socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        pthread_exit(NULL); // Kết thúc thread nếu lỗi
    }

    // Kết nối đến server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        pthread_exit(NULL); // Kết thúc thread nếu lỗi
    }

    // Gửi HTTP request
    send(sock, request, strlen(request), 0);

    // Nhận phản hồi từ server
    ssize_t bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate dữ liệu nhận được
        // In phản hồi nếu cần (có thể bỏ để tiết kiệm thời gian)
        // printf("Response from server:\n%s\n", buffer);
    }

    // Đóng socket
    close(sock);
    pthread_exit(NULL); // Kết thúc thread
}

int main() {
    pthread_t threads[TOTAL_REQUESTS]; // Mảng chứa thread ID
    struct timeval start, end;         // Đo thời gian tổng cộng

    // Đo thời gian bắt đầu
    gettimeofday(&start, NULL);

    // Tạo các thread để gửi request
    for (int i = 0; i < TOTAL_REQUESTS; i++) {
        if (pthread_create(&threads[i], NULL, send_request, NULL) != 0) {
            perror("Error creating thread");
        }
    }

    // Chờ tất cả các thread hoàn thành
    for (int i = 0; i < TOTAL_REQUESTS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Đo thời gian kết thúc
    gettimeofday(&end, NULL);

    // Tính thời gian thực hiện
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    float total_elapsed = seconds * 1000 + microseconds / 1000.0; // Thời gian tính bằng ms

    // Tính throughput
    float throughput = (TOTAL_REQUESTS / total_elapsed) * 1000; // Requests per second

    //printf("\nTotal time for %d concurrent requests: %.2f ms\n", TOTAL_REQUESTS, total_elapsed);
    printf("\nTotal time taken for all files: %f ms\n", total_elapsed);
    printf("Throughput: %f requests per second\n\n", throughput);

    return 0;
}

