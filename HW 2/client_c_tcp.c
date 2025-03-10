//ChatGPT was used to write the a base TCP client program - I modified it to fit the needs of the lab

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 129

int main(int argc, char *argv[]) {
    //Input address and port
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];  // Get IP from command line
    int port = atoi(argv[2]);   // Convert port argument to integer
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&server_ip, 0, sizeof(server_ip)); 
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    //Prompt user for input
    printf("Enter string: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character

    // Send message to server
    write(sock, buffer, strlen(buffer)); 

    // Receive response from server
    while (1) { //Keep recieving responses until value is only one digit    
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(sock, buffer, BUFFER_SIZE - 1); //Read from server
        if (bytes_read <= 0) {
            break;  // Exit loop if server closes connection
        }
        buffer[bytes_read] = '\0'; 
        printf("From server: %s\n", buffer); //Print out response

        //Check if condition to break loop has been met
        if (strcmp(buffer, "Sorry, cannot compute!") == 0 || (strlen(buffer) == 1 && buffer[0] >= '0' && buffer[0] <= '9')) {
            break;
        }
    }

    //printf("Connection closed by server.\n");
    
    // Close socket and terminate program
    close(sock);
    return 0;
}
