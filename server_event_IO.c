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

#define PORT 8080
#define MAX_CLIENTS 1000

void send_file(int client_sock, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        const char *error_response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\n";
        send(client_sock, error_response, strlen(error_response), 0);
        return;
    }

    // Tạo header phản hồi
    char response_header[1024];
    snprintf(response_header, sizeof(response_header),
             "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n");

    send(client_sock, response_header, strlen(response_header), 0);

    // Gửi nội dung file
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_sock, buffer, bytes_read, 0);
    }

    fclose(file);
}

void handle_client(int client_sock) {
    char buffer[1024];
    ssize_t bytes_received;
    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';  // Null-terminate the received data
        printf("Received message: %s\n", buffer);

        // Phân tích request để xác định loại yêu cầu
        if (strstr(buffer, "GET / HTTP/1.1") != NULL) {
            // Yêu cầu Non-IO
            const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHello World\n";
            send(client_sock, response, strlen(response), 0);
        } else if (strstr(buffer, "GET /file HTTP/1.1") != NULL) {
            // Yêu cầu IO, trả về file
            send_file(client_sock, "example.txt"); // Đọc và trả về nội dung file example.txt
        } else {
            const char *response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\nBad Request\n";
            send(client_sock, response, strlen(response), 0);
        }
    }

    if (bytes_received <= 0) {
        close(client_sock);  // Đóng kết nối khi client đóng
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set readfds;
    int client_sockets[MAX_CLIENTS];

    // Tạo server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error creating socket");
        exit(1);
    }

    // Cấu hình địa chỉ server
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Error listening on socket");
        close(server_sock);
        exit(1);
    }

    // Set server socket to non-blocking mode
    fcntl(server_sock, F_SETFL, O_NONBLOCK);

    printf("Server is listening on port %d...\n", PORT);

    // Khởi tạo mảng client_sockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;  // Chưa có kết nối
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);  // Thêm server socket vào tập readfds

        // Thêm các client sockets vào readfds
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                FD_SET(client_sockets[i], &readfds);
            }
        }

        // Chờ sự kiện (kết nối mới hoặc dữ liệu đến từ client)
        printf("Waiting for events...\n");
        int activity = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Error in select");
            continue;
        }

        // Kiểm tra nếu có kết nối mới đến server
        if (FD_ISSET(server_sock, &readfds)) {
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
            if (client_sock < 0) {
                perror("Error accepting connection");
                continue;
            }
            printf("New connection accepted: client_socket %d\n", client_sock);

            // Chấp nhận kết nối và thêm vào mảng client_sockets
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == -1) {
                    client_sockets[i] = client_sock;
                    break;
                }
            }

            // Đặt client socket ở chế độ không chặn
            fcntl(client_sock, F_SETFL, O_NONBLOCK);
        }

        // Kiểm tra tất cả các client đang kết nối
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1 && FD_ISSET(client_sockets[i], &readfds)) {
                handle_client(client_sockets[i]);
                client_sockets[i] = -1;  // Đóng socket client sau khi xử lý
            }
        }
    }

    close(server_sock);
    return 0;
}
