#include "helper.h"
#include "client_thread.h"

int debug = 1;
int max_username_length = 50;
int max_message_length = 1024;
int max_message_buffer = 10;

int main(int argc, char* argv[]) {
    if(argc != 4) {
        fprintf(stderr, "Usage: %s <port> <max_clients> <timeout>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int max_clients = atoi(argv[2]);
    int timeout = atoi(argv[3]);

    int server_fd = create_socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address = bind_socket(AF_INET, INADDR_ANY, port, server_fd);
    int server_addrlen = sizeof(server_address);

    if(listen(server_fd, max_clients) < 0) {
        fprintf(stderr, "Unable to listen for connections :(\n");
        exit(EXIT_FAILURE);
    }

    if(debug)
        fprintf(stdout, "Server listening on port %d\n", port);

    pthread_t setup_thread;
    struct setup_args* args = (struct setup_args*) malloc(sizeof(struct setup_args));
    args->max_clients = max_clients;
    args->timeout = timeout;

    if(pthread_create(&setup_thread, NULL, setup, args) < 0) {
        fprintf(stderr, "Unable to create setup server :(\n");
        exit(EXIT_FAILURE);
    }

    pthread_t broadcast_thread;
    if(pthread_create(&broadcast_thread, NULL, broadcast_messages, &max_clients) < 0) {
        fprintf(stderr, "Unable to create broadcast thread :(\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        int client_fd;
        if((client_fd = accept(server_fd, (struct sockaddr *)&server_address, (socklen_t*)&server_addrlen)) < 0) {
            fprintf(stderr, "Unable to accept connection :(\n");
            exit(EXIT_FAILURE);
        }

        pthread_t client_thread;

        if(pthread_create(&client_thread, NULL, handle_client, & client_fd) < 0) {
            fprintf(stderr, "Unable to create client thread :(\n");
            exit(EXIT_FAILURE);
        }
    }
}