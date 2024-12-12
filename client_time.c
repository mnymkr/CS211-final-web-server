#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h> // For time measurement

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define TOTAL_REQUESTS 1000 // Tổng số requests

int main() {
    struct sockaddr_in server_addr;
    char *request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    char buffer[BUFFER_SIZE];
    struct timeval start, end; // Để đo thời gian tổng cộng

    // Cấu hình địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Đo thời gian bắt đầu
    gettimeofday(&start, NULL);

    for (int i = 0; i < TOTAL_REQUESTS; i++) {
        int sock;

        // Tạo socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }

        // Kết nối đến server
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Connection failed");
            close(sock);
            continue; // Bỏ qua request này và tiếp tục
        }

        // Gửi HTTP request
        send(sock, request, strlen(request), 0);

        // Nhận phản hồi từ server
        ssize_t bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            perror("Error receiving response");
        } else {
            buffer[bytes_received] = '\0'; // Null-terminate dữ liệu nhận được
            // In phản hồi nếu cần (có thể bỏ để tiết kiệm thời gian)
            // printf("Response from server:\n%s\n", buffer);
        }

        // Đóng socket
        close(sock);
    }

    // Đo thời gian kết thúc
    gettimeofday(&end, NULL);

    // Tính thời gian thực hiện
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    float total_elapsed = seconds * 1000 + microseconds / 1000.0; // Thời gian tính bằng ms

    // Tính throughput
    float throughput = (TOTAL_REQUESTS / total_elapsed) * 1000; // Requests per second

    printf("\nTotal time for %d requests: %.2f ms\n", TOTAL_REQUESTS, total_elapsed);
    printf("Throughput: %.2f requests per second\n\n", throughput);

    return 0;
}

