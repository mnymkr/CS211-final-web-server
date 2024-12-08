#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 1500 // Maximum number of concurrent threads
#define SERVER_IP "127.0.0.1"

typedef struct {
    char filepath[BUFFER_SIZE];
} request_t;

void *send_file_request(void *arg) {
    request_t *request = (request_t *)arg;
    struct timeval start, end;

    // Start timing
    gettimeofday(&start, NULL);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        free(request); // Free allocated memory for request
        return NULL;
    }

    // Set up the server address structure
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(server_socket);
        free(request); // Free allocated memory for request
        return NULL;
    }

    // Send the filepath to the server
    send(server_socket, request->filepath, strlen(request->filepath), 0);

    // Receive and print the file content from the server
    char buffer[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(server_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received string
        // printf("%s", buffer); // Print the received content
    }

    close(server_socket);

    // End timing
    gettimeofday(&end, NULL);
    
    // Calculate elapsed time in milliseconds
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    long elapsed = seconds * 1000 + microseconds / 1000;

    printf("\nTime taken to process %s: %ld milliseconds\n", request->filepath, elapsed);

    free(request); // Free allocated memory for request
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *directory_path = argv[1];

    DIR *dir;
    struct dirent *entry;   // pointer to the each directory entry (file)

    // Open the directory
    dir = opendir(directory_path);
    pthread_t threads[BACKLOG]; // Array to hold thread IDs
    int thread_count = 0;
    int total_request = 0;

    struct timeval total_start, total_end;

    // Start total timing
    gettimeofday(&total_start, NULL);

    // Read files from the directory and create threads for each request
    while ((entry = readdir(dir)) != NULL) {
        // Skip the current and parent directory entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            request_t *request = malloc(sizeof(request_t));

            // Assign values to the request filepath.
            snprintf(request->filepath, sizeof(request->filepath), "%s/%s", directory_path, entry->d_name);
            // printf("Requesting file: %s\n", request->filepath);

            // Create a new thread for each file request
            if (pthread_create(&threads[thread_count], NULL, send_file_request, request) != 0) {
                perror("Could not create thread");
                free(request); // Free allocated memory on error
                continue; // Skip this iteration on error
            }
            thread_count++;
            total_request++;
            
            // Limit number of concurrent threads to avoid resource exhaustion.
            if (thread_count >= BACKLOG) {
                for (int i = 0; i < thread_count; i++) {
                    pthread_join(threads[i], NULL);
                }
                thread_count = 0; // Reset thread count after joining.
            }
        }
    }

    // Wait for any remaining threads to finish.
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    closedir(dir);

    // End total timing
    gettimeofday(&total_end, NULL);

    long total_seconds = total_end.tv_sec - total_start.tv_sec;
    long total_microseconds = total_end.tv_usec - total_start.tv_usec;
    
    float total_elapsed = total_seconds * 1000 + total_microseconds / 1000.0;
    printf("\nTotal time taken for all files: %f ms\n", total_elapsed);
    printf("Total requests: %d\n", total_request);

    float throughput = (float)total_request/(total_elapsed/1000.0);
    printf("Throughput: %f requests per second\n\n", throughput);

    return EXIT_SUCCESS;
}