//ChatGPT was used to write the a base UDP server program - I modified it to fit the needs of the lab

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
    int server_fd; 
    struct sockaddr_in server_address, client_address;
    socklen_t len = sizeof(client_address); 
    char buffer[BUFFER_SIZE] = {0};

    memset(buffer, '\0', BUFFER_SIZE); 

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_fd); 
        exit(EXIT_FAILURE);
    }

    while(1){
        memset(buffer, '\0', BUFFER_SIZE); 
        int end = recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_address, &len); //Recieve message from client
        buffer[end] = '\0'; 

        //Make sure message is a valid input (i.e. an integer)
        int validInput = 1; 
        for(int i = 0; buffer[i] != '\0'; i++) 
        {
            if(!isdigit(buffer[i]))
            {
                validInput = 0; 
            }
        }

        if(validInput == 0) //If message is invalid (i.e. contains text), return cannot compute message
        {
            const char* response = "Sorry, cannot compute!"; 
            sendto(server_fd, response, strlen(response), 0, (const struct sockaddr *)&client_address, len); 
            continue; 
        }
        else
        {
            char bufferCpy[BUFFER_SIZE]; 
            strcpy(bufferCpy, buffer); 
            while(1){ //Repeat until value is a single digit
                int sum = 0; 
                for(int i = 0; bufferCpy[i] != '\0'; i++) //Calculate sum of digits
                {
                    sum += bufferCpy[i] - '0'; 
                }
                sprintf(bufferCpy, "%d", sum); 
                sendto(server_fd, bufferCpy, strlen(bufferCpy), 0, (const struct sockaddr *)&client_address, len); //Send result to client

                if(sum < 10) //End loop if value is a single digit
                {
                    break; 
                }
            }
        }

    }
    
    close(server_fd);
    return 0;
}
