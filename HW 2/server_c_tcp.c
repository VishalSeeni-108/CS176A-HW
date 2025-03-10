//ChatGPT was used to write the a base TCP server program - I modified it to fit the needs of the lab

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 129

int main(int argc, char *argv[]) {
    //Input port
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);  // Convert port argument to integer
    int server_fd, client_fd; 
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address); 
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { 
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Define server address
    memset(&server_address, 0, sizeof(server_address)); 
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    //printf("Server listening on port %d...\n", port);

    while(1) //Enter loop - repeat until output is only one digit
    {
        // Accept a client connection
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

         //printf("Client connected.\n");

        // Receive data from client
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);

        if (bytes_received <= 0) { //If no data is recieved, close connection
            //printf("Client disconnected.\n");
            close(client_fd);
            continue; 
         }

         buffer[bytes_received] = '\0'; 

        //Check if input is valid (i.e. all numbers)
         int validInput = 1; 
         for(int i = 0; buffer[i] != '\0'; i++){
            if(!isdigit(buffer[i])){
                validInput = 0; 
            }
         }
         if(validInput == 0) //If input is not valid, return cannot compute message
         {
            const char *response = "Sorry, cannot compute!"; 
            write(client_fd, response, strlen(response)); 
         }
         else
         {
            char bufferCpy[BUFFER_SIZE]; 
            strcpy(bufferCpy, buffer); 
            while(1){
                //Calculate sum of digits
                int sum = 0;
                for(int i = 0; bufferCpy[i] != '\0'; i++){
                    sum += bufferCpy[i] - '0'; 
                }
                sprintf(bufferCpy, "%d", sum); 
                write(client_fd, bufferCpy, strlen(bufferCpy)); //Send result back to client

                if(sum < 10){ //If sum is only one digit, break loop
                    break; 
                }
                usleep(100000); // Sleep for 100ms in order to break up messages
            }
         }
         close(client_fd); 
    }
    close(server_fd); 
    return 0;
}
