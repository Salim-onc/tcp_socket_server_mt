// multi_server.c
// Complete TCP Multi-Threaded Echo Server in C (POSIX threads)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Get client IP for logging
    if (getpeername(client_sock, (struct sockaddr*)&client_addr, &addr_len) == 0) {
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    } else {
        strcpy(client_ip, "unknown");
    }

    printf("[+] Client connected: %s (socket %d)\n", client_ip, client_sock);

    // Send welcome message
    const char* welcome = "=== Welcome to the Multi-Threaded TCP Echo Server ===\n"
                          "Send any message - it will be echoed back.\n"
                          "Type 'quit' or 'exit' (or just close connection) to disconnect.\n\n";
    send(client_sock, welcome, strlen(welcome), 0);

    while (1) {
        int bytes_received = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            // Client disconnected or error
            break;
        }

        buffer[bytes_received] = '\0';

        // Log received message
        printf("[RECV from %s]: %s", client_ip, buffer);

        // Echo the message back exactly as received
        if (send(client_sock, buffer, bytes_received, 0) < 0) {
            perror("[-] Send failed");
            break;
        }

        // Check for exit command (case-insensitive, works with or without \r\n)
        if (strncasecmp(buffer, "quit", 4) == 0 || strncasecmp(buffer, "exit", 4) == 0) {
            printf("[-] Client %s requested disconnect\n", client_ip);
            break;
        }
    }

    printf("[-] Client disconnected: %s (socket %d)\n", client_ip, client_sock);
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // Create socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("[-] Socket creation failed");
        exit(1);
    }

    // Allow immediate reuse of address (useful during development)
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[-] setsockopt failed");
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-] Bind failed");
        close(server_sock);
        exit(1);
    }

    // Listen
    if (listen(server_sock, 15) < 0) {
        perror("[-] Listen failed");
        close(server_sock);
        exit(1);
    }

    printf("[*] TCP Multi-Threaded Echo Server started on port %d\n", PORT);
    printf("[*] Waiting for clients... (Ctrl+C to stop)\n\n");

    // Main accept loop
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("[-] Accept failed");
            continue;  // Don't crash the server on transient errors
        }

        // Create a copy of the socket descriptor for the thread
        int* new_sock = malloc(sizeof(int));
        if (new_sock == NULL) {
            perror("[-] malloc failed");
            close(client_sock);
            continue;
        }
        *new_sock = client_sock;

        // Create detached thread (no need to join)
        if (pthread_create(&thread_id, NULL, handle_client, (void*)new_sock) != 0) {
            perror("[-] pthread_create failed");
            close(client_sock);
            free(new_sock);
            continue;
        }
        pthread_detach(thread_id);
    }

    close(server_sock);
    return 0;
}
