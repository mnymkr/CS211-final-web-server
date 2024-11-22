//
//  main.c
//  Client_multi
//
//  Created by Lê Giang on 22/11/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define NUM_THREADS 10
#define REQUESTS_PER_THREAD 10

void *send_requests(void *arg) {
    int thread_id = *(int *)arg;
    free(arg);

    int sock = 0;
    struct sockaddr_in serv_addr;
    char response[BUFFER_SIZE] = {0};

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        pthread_exit(NULL);
    }

    // Địa chỉ server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(sock);
        pthread_exit(NULL);
    }

    // Kết nối tới server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        pthread_exit(NULL);
    }

    // Gửi REQUESTS_PER_THREAD requests
    for (int i = 0; i < REQUESTS_PER_THREAD; ++i) {
        const char *request = "Hello";
        if (send(sock, request, strlen(request), 0) < 0) {
            perror("Send failed");
            close(sock);
            pthread_exit(NULL);
        }

        memset(response, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(sock, response, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Receive failed");
            close(sock);
            pthread_exit(NULL);
        }
        response[bytes_received] = '\0';
        printf("Thread %d - Response %d: %s\n", thread_id, i + 1, response);
    }

    close(sock);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    struct timeval start, end;

    gettimeofday(&start, NULL);

    // Tạo NUM_THREADS thread
    for (int i = 0; i < NUM_THREADS; ++i) {
        int *thread_id = malloc(sizeof(int));
        *thread_id = i + 1;
        pthread_create(&threads[i], NULL, send_requests, thread_id);
    }

    // Chờ tất cả các thread hoàn thành
    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;

    printf("Total time for %d requests: %.3f seconds\n", NUM_THREADS * REQUESTS_PER_THREAD, elapsed);
    printf("Throughput: %.3f requests/second\n", (NUM_THREADS * REQUESTS_PER_THREAD) / elapsed);

    return 0;
}

