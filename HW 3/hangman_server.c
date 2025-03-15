#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <sys/select.h>

#define MAX_CLIENTS 3
#define MAX_WORD_LENGTH 8
#define MAX_INCORRECT_GUESSES 6
#define PORT 8080
#define MAX_WORDS 15
#define MAX_BUFFER_SIZE 256

typedef struct {
    int socket;
    char word[MAX_WORD_LENGTH + 1];
    char display[MAX_WORD_LENGTH + 1];
    char incorrect_guesses[MAX_INCORRECT_GUESSES + 1];
    int incorrect_count;
    int in_use;
    int game_over;
} ClientState;

void load_words(char words[][MAX_WORD_LENGTH + 1], int *count) {
    FILE *file = fopen("hangman_words.txt", "r");
    if (!file) {
        perror("Error opening hangman_words.txt");
        exit(EXIT_FAILURE);
    }
    *count = 0;
    while (fscanf(file, "%s", words[*count]) != EOF && *count < MAX_WORDS) {
        int len = strlen(words[*count]);
        if (len >= 3 && len <= 8) {
            (*count)++;
        }
    }
    fclose(file);
    if (*count == 0) {
        fprintf(stderr, "Error: No valid words found in hangman_words.txt\n");
        exit(EXIT_FAILURE);
    }
}

char *get_random_word(char words[][MAX_WORD_LENGTH + 1], int count) {
    return words[rand() % count];
}

void initialize_client(ClientState *client, int client_socket, char *word) {
    client->socket = client_socket;
    strcpy(client->word, word);
    memset(client->display, '_', strlen(word));
    client->display[strlen(word)] = '\0';
    memset(client->incorrect_guesses, 0, sizeof(client->incorrect_guesses));
    client->incorrect_count = 0;
    client->in_use = 1;
    client->game_over = 0;
}

void send_game_state(int client_socket, char *display, char *incorrect_guesses, int word_length, int incorrect_count) {
    char buffer[MAX_BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%c%c%c%s%s", 0, (char)word_length, (char)incorrect_count, display, incorrect_guesses);
    send(client_socket, buffer, word_length + incorrect_count + 3, 0);
}

void send_message(int client_socket, char *message) {
    char buffer[MAX_BUFFER_SIZE];
    int message_length = strlen(message);
    buffer[0] = (char)message_length;
    memcpy(buffer + 1, message, message_length);
    send(client_socket, buffer, message_length + 1, 0);
}

void handle_client(ClientState *client) {
    if (client->game_over) {
        return;
    }

    char guess;
    int bytes_received = recv(client->socket, &guess, 1, MSG_DONTWAIT);

    if (bytes_received > 0) {
        guess = tolower(guess);
        //printf("Received guess %c\n", guess);

        int correct = 0;
        for (int i = 0; i < strlen(client->word); i++) {
            if (client->word[i] == guess) {
                client->display[i] = guess;
                correct = 1;
            }
        }

        if (!correct && strchr(client->incorrect_guesses, guess) == NULL) {
            client->incorrect_guesses[client->incorrect_count] = guess;
            client->incorrect_count++;
        }

        if (strcmp(client->word, client->display) == 0) {
            char win_message[MAX_BUFFER_SIZE];
            snprintf(win_message, sizeof(win_message), "The word was %s", client->word);
            send_message(client->socket, win_message);
            usleep(1); 
            send_message(client->socket, "You Win!");
            usleep(1); 
            send_message(client->socket, "Game Over!");
            client->game_over = 1;
        } else if (client->incorrect_count >= MAX_INCORRECT_GUESSES) {
            char lose_message[MAX_BUFFER_SIZE];
            snprintf(lose_message, sizeof(lose_message), "The word was %s", client->word);
            send_message(client->socket, lose_message);
            usleep(1); 
            send_message(client->socket, "You Lose!");
            usleep(1); 
            send_message(client->socket, "Game Over!");
            client->game_over = 1;
        }

        send_game_state(client->socket, client->display, client->incorrect_guesses, strlen(client->word), client->incorrect_count);

        if (client->game_over) {
            close(client->socket);
            client->in_use = 0;
        }
    } else if (bytes_received == 0) {
        client->in_use = 0;
        client->game_over = 1;
        close(client->socket);
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char words[MAX_WORDS][MAX_WORD_LENGTH + 1];
    int word_count;
    ClientState clients[MAX_CLIENTS] = {0};

    srand(time(NULL));
    load_words(words, &word_count);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);

    fd_set readfds;
    int max_sd, sd;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket;
            if (clients[i].in_use) {
                FD_SET(sd, &readfds);
                if (sd > max_sd) {
                    max_sd = sd;
                }
            }
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("select error");
            continue;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (client_socket >= 0) {
                int client_index = -1;
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (!clients[i].in_use) {
                        client_index = i;
                        break;
                    }
                }

                if (client_index == -1) {
                    send_message(client_socket, "server-overloaded");
                    close(client_socket);
                } else {
                    send_message(client_socket, "server-connected");
                    load_words(words, &word_count);
                    char *word = get_random_word(words, word_count);
                    initialize_client(&clients[client_index], client_socket, word);
                    send_game_state(clients[client_index].socket, clients[client_index].display, clients[client_index].incorrect_guesses, strlen(clients[client_index].word), clients[client_index].incorrect_count);
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket;
            if (clients[i].in_use && FD_ISSET(sd, &readfds)) {
                handle_client(&clients[i]);
            }
        }
    }
    return 0;
}