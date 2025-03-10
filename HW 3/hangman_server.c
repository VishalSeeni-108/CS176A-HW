#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>

#define PORT 8080
#define MAX_CLIENTS 3
#define BUFFER_SIZE 128
#define MAX_WORDS 15
#define MAX_INCORRECT 6

typedef struct {
    int socket;
    char word[9];
    char display[9];
    char incorrect[7];
    int incorrect_count;
} ClientData;

char words[MAX_WORDS][9];
int word_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void load_words() {
    FILE *file = fopen("hangman_words.txt", "r");
    if (!file) {
        perror("Error opening word file");
        exit(EXIT_FAILURE);
    }
    
    while (fgets(words[word_count], sizeof(words[word_count]), file)) {
        words[word_count][strcspn(words[word_count], "\n")] = '\0';
        word_count++;
        if (word_count >= MAX_WORDS) break;
    }
    
    fclose(file);
}

char* choose_random_word() {
    return words[rand() % word_count];
}

void send_game_state(ClientData *client, int msg_flag, const char *message) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    if (msg_flag > 0) {
        buffer[0] = msg_flag;
        strcpy(buffer + 1, message);
        send(client->socket, buffer, msg_flag + 1, 0);
    } else {
        buffer[0] = 0;
        buffer[1] = strlen(client->word);
        buffer[2] = client->incorrect_count;
        strcpy(buffer + 3, client->display);
        strcat(buffer + 3 + strlen(client->word), client->incorrect);
        send(client->socket, buffer, BUFFER_SIZE, 0);
    }
}

void *handle_client(void *arg) {
    ClientData client = *(ClientData *)arg;
    char buffer[BUFFER_SIZE];
    
    recv(client.socket, buffer, BUFFER_SIZE, 0);
    send_game_state(&client, 0, NULL);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client.socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;

        char guess = tolower(buffer[0]);
        int correct = 0;

        for (int i = 0; i < strlen(client.word); i++) {
            if (client.word[i] == guess) {
                client.display[i] = guess;
                correct = 1;
            }
        }

        if (!correct) {
            client.incorrect[client.incorrect_count++] = guess;
        }

        if (client.incorrect_count >= MAX_INCORRECT) {
            send_game_state(&client, 8, "You Lose :(");
            break;
        }

        if (strcmp(client.word, client.display) == 0) {
            send_game_state(&client, 8, "You Win!");
            break;
        }

        send_game_state(&client, 0, NULL);
    }
    
    send_game_state(&client, 8, "Game Over!");
    close(client.socket);
    free(arg);
    return NULL;
}

int main() {
    srand(time(NULL));
    load_words();
    
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Hangman Server running on port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

        if (client_socket == -1) {
            perror("Client connection failed");
            continue;
        }

        pthread_mutex_lock(&lock);
        ClientData *client_data = malloc(sizeof(ClientData));
        client_data->socket = client_socket;
        strcpy(client_data->word, choose_random_word());
        memset(client_data->display, '_', strlen(client_data->word));
        client_data->display[strlen(client_data->word)] = '\0';
        memset(client_data->incorrect, 0, 7);
        client_data->incorrect_count = 0;
        pthread_mutex_unlock(&lock);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_data);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
