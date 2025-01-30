#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 129

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);  // Convert port argument to integer
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);

    // Accept a client connection
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Receive data from client
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = read(new_socket, buffer, BUFFER_SIZE);
    
    if (bytes_received <= 0) {
        printf("Client disconnected.\n");
        close(new_socket);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Process received data
    char response[128] = "";
    int validInput = 1;
    int sum = 0;

    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer[i] == '\n') {
            break;
        } else if (isalpha(buffer[i])) {
            strcpy(response, "Sorry, cannot compute");
            validInput = 0;
            break;
        } else if (isdigit(buffer[i])) {
            sum += buffer[i] - '0';
        }
    }

    if (validInput) {
        sprintf(response, "%d", sum);
    }

    // Send response
    write(new_socket, response, strlen(response));

    // Continue sending reduced sum until it becomes a single digit
    while (sum > 9) {
        int new_sum = 0, temp = sum;

        while (temp != 0) {
            new_sum += temp % 10;
            temp /= 10;
        }

        sum = new_sum;
        sprintf(response, "%d", sum);
        write(new_socket, response, strlen(response));
        usleep(100000); // Sleep for 100ms (optional)
    }

    printf("Server finished processing. Closing connection.\n");

    // Properly close the connection and terminate the program
    close(new_socket);
    close(server_fd);
    return 0;
}
