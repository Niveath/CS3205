#include "client_thread.h"
#include "helper.h"

int num_connected_clients = 0;
int num_messages = 0;
int max_clients;
int timeout;

struct client_message* messages_head = NULL, *messages_tail = NULL;
struct client_args* client_head = NULL, *client_tail = NULL;

pthread_mutex_t client_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER, full = PTHREAD_COND_INITIALIZER;

void* setup(void* args) {
    struct setup_args* setup_args = (struct setup_args*) args;
    max_clients = setup_args->max_clients;
    timeout = setup_args->timeout;
}

void* handle_client(void* args) {
    int client_fd = *((int*) args);

    // check if the server is full
    pthread_mutex_lock(&client_count_mutex);
    if(num_connected_clients < max_clients) {
        char* message = "Please enter your username: ";
        send(client_fd, message, strlen(message), 0);
    }
    else {
        char* message = "Server is full. Please try again later\n";
        send(client_fd, message, strlen(message), 0);
        close(client_fd);
        pthread_mutex_unlock(&client_count_mutex);
        return NULL;
    }
    pthread_mutex_unlock(&client_count_mutex);

    // read username from client
    char* username = (char*) malloc(max_username_length);
    struct client_args* client = (struct client_args*) malloc(sizeof(struct client_args));
    client->client_fd = client_fd;
    client->username = username;
    client->next = NULL;

    int valread = read(client_fd, client->username, max_username_length);
    if(valread < 0) {
        fprintf(stderr, "Unable to read username from client :(\n");
        close(client_fd);
        return NULL;
    }

    client->username[valread] = '\0';

    // check if the username is already taken and add client to client list
    pthread_mutex_lock(&client_mutex);
    if(client_head == NULL) {
        client_head = client;
        client_tail = client;
    } else {
        struct client_args* temp = client_head;
        while(temp != NULL) {
            if(strcmp(temp->username, client->username) == 0) {
                char* message = "Username already taken. Please try again: ";
                send(client_fd, message, strlen(message), 0);
                close(client_fd);
                pthread_mutex_unlock(&client_mutex);
                free(client->username);
                free(client);
                return NULL;
            }
            temp = temp->next;
        }
        client_tail->next = client;
        client_tail = client;
    }

    pthread_mutex_lock(&client_count_mutex);
    num_connected_clients++;
    pthread_mutex_unlock(&client_count_mutex);
    
    pthread_mutex_unlock(&client_mutex);

    if(debug)
        fprintf(stdout, "Client connected with username %s, fd:%d\n", client->username, client->client_fd);

    // send welcome message to client
    char* welcome_message = (char*) malloc(max_message_length);
    sprintf(welcome_message, "Welcome to The ChatRoom, %s!\nCurrently online users:\n", client->username);
    strcat(welcome_message, get_current_users(client->client_fd));
    send(client->client_fd, welcome_message, strlen(welcome_message), 0);
    free(welcome_message);

    // send message that client has joined to other clients
    struct client_message* message = (struct client_message*) malloc(sizeof(struct client_message));
    message->sender_fd = client->client_fd;
    message->sender_username = client->username;
    message->message = (char*) malloc(max_message_length);
    message->length = sprintf(message->message, "%s has joined the chatroom\n", client->username);
    message->next = NULL;

    pthread_mutex_lock(&message_mutex);
    while(num_messages == max_message_buffer)
        pthread_cond_wait(&empty, &message_mutex);
    if(messages_head == NULL) {
        messages_head = message;
        messages_tail = message;
    }
    else {
        messages_tail->next = message;
        messages_tail = message;
    }
    num_messages++;
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&message_mutex);

    int last_client_message = time(NULL);
    char* buffer = (char*) malloc(max_message_length);

    // create a thread to check for client timeout
    struct timeout_args* timeout_args = (struct timeout_args*) malloc(sizeof(struct timeout_args));
    timeout_args->client_fd = client->client_fd;
    timeout_args->is_timedout = 0;
    timeout_args->last_message = time(NULL);
    pthread_mutex_init(&timeout_args->timeout_mutex, NULL);

    pthread_t timeout_thread;
    if(pthread_create(&timeout_thread, NULL, check_timeout, timeout_args) < 0) {
        fprintf(stderr, "Unable to create timeout thread :(\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        int valread = read(client->client_fd, buffer, max_message_length);
        if(valread <= 0) {
            break;
        }
        buffer[valread] = '\0';

        if(debug)
            fprintf(stdout, "Received message from %s: %s\n", client->username, buffer);

        if(strcmp(buffer, "\\bye") == 0) {
            break;
        }
        else if(strcmp(buffer, "\\list") == 0) {
            char* users = get_current_users(client->client_fd);
            send(client->client_fd, users, strlen(users), 0);
            continue;
        }

        message = (struct client_message*) malloc(sizeof(struct client_message));
        message->sender_fd = client->client_fd;
        message->sender_username = client->username;
        message->message = (char*) malloc(max_message_length);
        message->length = sprintf(message->message, "%s: %s", client->username, buffer);
        message->next = NULL;

        pthread_mutex_lock(&message_mutex);
        while(num_messages == max_message_buffer)
            pthread_cond_wait(&empty, &message_mutex);
        if(messages_head == NULL) {
            messages_head = message;
            messages_tail = message;
        }
        else {
            messages_tail->next = message;
            messages_tail = message;
        }
        num_messages++;
        pthread_cond_signal(&full);
        pthread_mutex_unlock(&message_mutex);
    }

    //send message that client has disconnected to other clients
    message = (struct client_message*) malloc(sizeof(struct client_message));
    message->sender_fd = client->client_fd;
    message->sender_username = client->username;
    message->message = (char*) malloc(max_message_length);
    message->length = sprintf(message->message, "%s has left the ChatRoom\n", client->username);

    pthread_mutex_lock(&message_mutex);
    while(num_messages == max_message_buffer)
        pthread_cond_wait(&empty, &message_mutex);
    if(messages_head == NULL) {
        messages_head = message;
        messages_tail = message;
    }
    else {
        messages_tail->next = message;
        messages_tail = message;
    }
    num_messages++;
    pthread_cond_signal(&full);
    pthread_mutex_unlock(&message_mutex);

    // if the client timeouted, send a error message to the client
    pthread_mutex_lock(&timeout_args->timeout_mutex);
    if(timeout_args->is_timedout) {
        pthread_mutex_unlock(&timeout_args->timeout_mutex);
        return 0;
        char* message = "You have been disconnected due to inactivity\n";
        send(client->client_fd, message, strlen(message), 0);
    }
    pthread_mutex_unlock(&timeout_args->timeout_mutex);

    // remove client from client list
    pthread_mutex_lock(&client_mutex);
    struct client_args* temp = client_head;
    if(temp->client_fd == client->client_fd) {
        client_head = client_head->next;
        free(temp);
    }
    else {
        while(temp->next != NULL) {
            if(temp->next->client_fd == client->client_fd) {
                struct client_args* temp2 = temp->next;
                temp->next = temp2->next;
                free(temp2);
                break;
            }
            temp = temp->next;
        }
    }

    if(client_head == NULL)
        client_tail = NULL;

    pthread_mutex_unlock(&client_mutex);

    pthread_mutex_lock(&client_count_mutex);
    num_connected_clients--;
    pthread_mutex_unlock(&client_count_mutex);

    if(debug)
        fprintf(stdout, "Closing fd %d. Client %s disconnected\n", client_fd, username);
    
    free(buffer);
    free(username);
    
    // close the socket if not already closed
    pthread_mutex_lock(&timeout_args->timeout_mutex);
    if(!timeout_args->is_timedout) {
        close(client_fd);
    }
    pthread_mutex_unlock(&timeout_args->timeout_mutex);
}

void* broadcast_messages(void* args) {
    max_clients = *((int*) args);

    while(1) {
        pthread_mutex_lock(&message_mutex);
        while(num_messages == 0)
            pthread_cond_wait(&full, &message_mutex);
        struct client_message* message = messages_head;
        while(message != NULL) {
            pthread_mutex_lock(&client_mutex);
            struct client_args* client = client_head;
            while(client != NULL) {
                if(client->client_fd != message->sender_fd) 
                    send(client->client_fd, message->message, message->length, 0);
                client = client->next;
            }
            pthread_mutex_unlock(&client_mutex);

            struct client_message* temp = message;

            message = message->next;

            free(temp->message);
            free(temp);
        }

        messages_head = NULL;
        messages_tail = NULL;
        num_messages = 0;

        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&message_mutex);
    }
}

void* check_timeout(void* args) {
    struct timeout_args* timeout_args = (struct timeout_args*) args;

    while(1) {
        time_t current_time = time(NULL);

        pthread_mutex_lock(&timeout_args->timeout_mutex);
        if(current_time - timeout_args->last_message > timeout) {
            timeout_args->is_timedout = 1;
            char* message = "You have been disconnected due to inactivity\n";
            send(timeout_args->client_fd, message, strlen(message), 0);
            close(timeout_args->client_fd);

            if(debug)
                fprintf(stdout, "Client %d has been disconnected due to inactivity\n", timeout_args->client_fd);
                
            pthread_mutex_unlock(&timeout_args->timeout_mutex);
            break;
        }
        pthread_mutex_unlock(&timeout_args->timeout_mutex);
    }   
}

char* get_current_users(int client_fd) {
    char* users = (char*) malloc(max_username_length * max_clients);
    pthread_mutex_lock(&client_mutex);
    struct client_args* temp = client_head;
    while(temp != NULL) {
        if(temp->client_fd != client_fd) {
            strcat(users, temp->username);
            strcat(users, "\n");
        }
        else {
            strcat(users, temp->username);
            strcat(users, "(You)\n");
        }
        temp = temp->next;
    }
    pthread_mutex_unlock(&client_mutex);

    return users;
}