#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <limits.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);

    // Create UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", server_ip, port);

    // Communication loop
    while (1) {
        printf("Enter string: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  // Remove newline character

        // Send message to server
        sendto(sock, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr, addr_len);

        // Check for exit condition
        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }

            int currValue = INT_MAX; 
            while(currValue > 9)
            {
                memset(buffer, 0, BUFFER_SIZE);
                int bytes_received = recvfrom(sock, buffer, BUFFER_SIZE, 0,
                                                (struct sockaddr*)&server_addr, &addr_len);
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';  // Null-terminate the received string
                    printf("From server: %s\n", buffer);
                    currValue = atoi(buffer);
                } else {
                    break;
                }
            }     
    }

    close(sock);
    return 0;
}
