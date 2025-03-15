#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/select.h>

#define MAX_WORD_LENGTH 8
#define MAX_INCORRECT_GUESSES 6
#define MAX_BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, ">>>Usage: %s <server_ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int client_socket;
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror(">>>Socket creation failed");
        return EXIT_FAILURE;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        fprintf(stderr, ">>>Invalid address/Address not supported\n");
        close(client_socket);
        return EXIT_FAILURE;
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror(">>>Connection failed");
        close(client_socket);
        return EXIT_FAILURE;
    }

    char overloadCheck[MAX_BUFFER_SIZE];
    recv(client_socket, overloadCheck, sizeof(overloadCheck), 0); 
    if (strcmp(overloadCheck + 1, "server-overloaded") == 0) {
        printf(">>>server-overloaded\n"); 
        close(client_socket); 
        return 0;
    }
    else
    {

    }

    char ready;
    printf(">>>Ready to start game? (y/n): ");
    if (scanf(" %c", &ready) == EOF) {
        printf("\n");
        close(client_socket);
        return EXIT_SUCCESS;
    }
    while(getchar() != '\n'); //consume rest of line

    if (tolower(ready) != 'y') {
        close(client_socket);
        return EXIT_SUCCESS;
    }
    // Send empty message to start game
    char start_message[1] = {0};
    send(client_socket, start_message, 1, 0);

    char buffer[MAX_BUFFER_SIZE];
    int game_over = 0;

    // Check for "Server Overloaded" using select
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client_socket, &readfds);

    struct timeval tv;
    tv.tv_sec = 1; // Timeout after 1 second
    tv.tv_usec = 0;

    int retval = select(client_socket + 1, &readfds, NULL, NULL, &tv);
    if (retval > 0 && FD_ISSET(client_socket, &readfds)) {
        recv(client_socket, buffer, sizeof(buffer), 0);
        if (strcmp(buffer + 1, "Server Overloaded!") == 0) {
            printf(">>>Server Overloaded\n");
            close(client_socket);
            return EXIT_SUCCESS;
        }
    }

    while (!game_over) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            //perror(">>>Server disconnected");
            break;
        }

        if (buffer[0] > 0) { // Message packet
            printf(">>>%s\n", buffer + 1);
            if (strcmp(buffer + 1, "Game Over!") == 0) {
                game_over = 1;
                return 0;
            }
        } else { // Game control packet
            int word_length = (unsigned char)buffer[1];
            int incorrect_count = (unsigned char)buffer[2];
            char display[MAX_WORD_LENGTH + 1];
            char incorrect_guesses[MAX_INCORRECT_GUESSES + 1];

            strncpy(display, buffer + 3, word_length);
            display[word_length] = '\0';

            strncpy(incorrect_guesses, buffer + 3 + word_length, incorrect_count);
            incorrect_guesses[incorrect_count] = '\0';

            printf(">>>");
            for(int i = 0; i < word_length-1; i++){
                printf("%c ", display[i]);
            }
            printf("%c", display[word_length-1]); 
            printf("\n");

            printf(">>>Incorrect Guesses: ");
            for(int i = 0; i < incorrect_count-1; i++){
                printf("%c ", incorrect_guesses[i]);
            }
            if(incorrect_count >= 1)
            {
                printf("%c", incorrect_guesses[incorrect_count - 1]);
            }
            printf("\n>>>\n");

            if (!game_over) {
                char guess[MAX_BUFFER_SIZE]; // Use a larger buffer
                while (1) {
                    printf(">>>Letter to guess: ");
                    if (fgets(guess, sizeof(guess), stdin) == NULL) {
                        printf("\n");
                        close(client_socket);
                        return EXIT_SUCCESS;
                    }
                    guess[strcspn(guess, "\n")] = 0; // Remove newline

                    if (strlen(guess) != 1 || !isalpha(guess[0])) {
                        printf(">>>Error! Please guess one letter.\n");
                    } else {
                        guess[0] = tolower(guess[0]);
                        send(client_socket, guess, 1, 0);
                        break;
                    }
                }
            }
        }
    }

    close(client_socket);
    return EXIT_SUCCESS;
}