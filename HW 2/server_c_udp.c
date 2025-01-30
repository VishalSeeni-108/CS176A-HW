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
    //Actual bind statement bind(server_fd, (struct sockaddr*)&address, sizeof(address))
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    //printf("Server listening on port %d...\n", port);

    // Accept a client connection
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    //printf("Client connected.\n");

    // Communicate with client (Receive and Send messages in a loop)
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Client disconnected.\n");
            break;
        }
        //printf("Client: %s\n", buffer);
        char response[128] = "";
        int validInput = 1;
        int sum = 0;
        for(int i = 0; i < BUFFER_SIZE; i++)
        {
            if(buffer[i] == '\n')
            {
                break; 
            }
            else if(isalpha(buffer[i]))
            {
                strcpy(response, "Sorry, cannot compute\r\n"); 
                validInput = 0;
                break; 
            }
            else if(isdigit(buffer[i]))
            {
                sum += buffer[i] - '0'; 
            }
        }
        if(validInput == 1)
        {
            sprintf(response, "%d", sum); 
        }

        // Send response
        send(new_socket, response, strlen(response), 0);
        while(sum > 9)
        {
            int digits[256];
            int N = sum;
            int i = 0;
            int j, r; 
            while (N != 0) 
            { 
            // Extract the last digit of N 
            r = N % 10; 
    
            // Put the digit in arr[] 
            digits[i] = r; 
            i++; 
    
            // Update N to N/10 to extract 
            // next last digit 
            N = N / 10; 
            } 

            sum = 0; 
            for(int x = 0; x < i; x++)
            {
                sum += digits[x]; 
            }
            //printf("New sum: %d\n\r", sum); 
            sprintf(response, "%d", sum); 
            send(new_socket, response, strlen(response), 0);
            usleep(100000); 
        }
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
