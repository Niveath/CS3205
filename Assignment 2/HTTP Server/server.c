#include "helper.h"
#include "response.h"

#define MAX_CLIENTS 10

int debug = 1;
char* HTTP200 = "HTTP/1.1 200 OK\n\r";
char* HTTP404 = "HTTP/1.1 404 NOT FOUND\n\r";

int main(int argc, char* argv[]) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <port_number> <root_file_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int port_number = atoi(argv[1]);
    char* root_dir = argv[2];

    int server_fd, client_fd;
    char buffer[BUFFERSIZE] = {0};

    server_fd = create_socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address = bind_socket(AF_INET, INADDR_ANY, port_number, server_fd);
    int server_addrlen = sizeof(server_address);

    if(listen(server_fd, MAX_CLIENTS) < 0) {
        fprintf(stderr, "Unable to listen for connections :(\n");
        exit(EXIT_FAILURE);
    }

    if(debug)
        fprintf(stdout, "Server listening on port %d\n", port_number);

    while(1) {
        if((client_fd = accept(server_fd, (struct sockaddr*) &server_address, (socklen_t*)&server_addrlen)) < 0) {
            fprintf(stderr, "Unable to connect to client\n");
            exit(EXIT_FAILURE);
        }

        if(debug) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(server_address.sin_addr), client_ip, INET_ADDRSTRLEN);
            fprintf(stdout, "Client IP Address: %s\n", client_ip);
        }

        int valread = read(client_fd, buffer, BUFFERSIZE);
        if(valread <= 0) {
            if(valread == 0)
                fprintf(stderr, "Client disconnected\n");
            else if(valread < 0)
                fprintf(stderr, "Unable to read from client\n");
            close(client_fd);
            continue;
        }
        buffer[valread] = '\0';

        char* full_buffer = (char*) malloc((strlen(buffer) + 1)* sizeof(char));
        strcpy(full_buffer, buffer);

        if(debug)
            fprintf(stdout, "%s\n", buffer);

        char* request_type = strtok(buffer, " ");
        
        if(strcmp(request_type, "GET") == 0) {
            pthread_t get_response_thread;

            struct get_response* thread_response = (struct get_response*) malloc(sizeof(struct get_response));

            char* request_path = strtok(NULL, " ");

            thread_response->client_fd = client_fd;
            thread_response->root_dir = root_dir;
            thread_response->request_path = (char*) malloc((strlen(request_path) + 1)* sizeof(char));
            strcpy(thread_response->request_path, request_path);

            pthread_create(&get_response_thread, NULL, &get_response, thread_response);
        }
        else if(strcmp(request_type, "POST") == 0) {
            pthread_t post_response_thread;

            struct post_response* thread_response = (struct post_response*) malloc(sizeof(struct post_response));

            thread_response->client_fd = client_fd;
            thread_response->message = (char*) malloc((strlen(full_buffer) + 1)* sizeof(char));
            strcpy(thread_response->message, full_buffer);

            pthread_create(&post_response_thread, NULL, &post_response, thread_response);
        }
    }

    close(server_fd);
    return 0;
}