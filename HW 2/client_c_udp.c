//ChatGPT was used to write the a base UDP client program - I modified it to fit the needs of the lab

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <limits.h>

#define BUFFER_SIZE 129

int main(int argc, char *argv[]) {
    //Input address and port
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_address);

    memset(buffer, '\0', BUFFER_SIZE); 

    // Create UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //Define server address
    memset(&server_address, 0, sizeof(server_address)); 
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(server_ip); 


    //printf("Connected to server at %s:%d\n", server_ip, port);

    //Prompt user for input
    printf("Enter string: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = 0;  // Remove newline character

    // Send message to server
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&server_address, addr_len);

    //Enter response loop - repeat until output is a single digit
    while(1){
        int n = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_address, &addr_len); //Recieve message from server
        if(n <= 0){ //If message is empty (i.e. server has stopped sending messages), end loop and move on
            break; 
        }

        buffer[n] = '\0'; 
        printf("From server: %s\n", buffer); //Print response from server

        //Check if requirements to end loop have been met
        if (strcmp(buffer, "Sorry, cannot compute!") == 0 || 
            (strlen(buffer) == 1 && buffer[0] >= '0' && buffer[0] <= '9')) {
            break;
        }
    } 

    //close socket
    close(sock);
    return 0;
}
