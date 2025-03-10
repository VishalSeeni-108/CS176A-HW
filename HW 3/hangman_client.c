#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 128

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];

    printf(">>> Ready to start game? (y/n): ");
    char response;
    scanf(" %c", &response);
    if (response != 'y' && response != 'Y') {
        close(sock);
        exit(0);
    }

    buffer[0] = 0;
    send(sock, buffer, 1, 0);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock, buffer, BUFFER_SIZE, 0);

        if (buffer[0] > 0) {
            printf(">>> %s\n", buffer + 1);
            break;
        }

        printf(">>> ");
        for (int i = 0; i < buffer[1]; i++) {
            printf("%c ", buffer[3 + i]);
        }
        printf("\n>>> Incorrect Guesses: ");
        for (int i = 0; i < buffer[2]; i++) {
            printf("%c ", buffer[3 + buffer[1] + i]);
        }
        printf("\n");

        printf(">>> Letter to guess: ");
        char guess;
        scanf(" %c", &guess);
        buffer[0] = 1;
        buffer[1] = tolower(guess);
        send(sock, buffer, 2, 0);
    }

    close(sock);
    return 0;
}
