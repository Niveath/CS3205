#include "response.h"
#include "helper.h"

void* get_response(void* args) {
    struct get_response* resp = (struct get_response*) args;

    char full_path[strlen(resp->root_dir) + strlen(resp->request_path) + 1];
    strcpy(full_path, resp->root_dir);
    strcat(full_path, resp->request_path);

    if(strcmp(resp->request_path, "/") == 0) {
        resp->request_path = malloc(100 * sizeof(char));
        resp->request_path = "/index.html";
        send_get_response(200, resp);
    }
    else if(path_exists(full_path))
        send_get_response(200, resp);
    else {
        resp->request_path = malloc(100 * sizeof(char));
        resp->request_path = "/404.html";
        send_get_response(404, resp);
    }
}

void* post_response(void* args) {
    struct post_response* resp = (struct post_response*) args;

    if(debug) {
        fprintf(stdout, "POST Request\n");
        fprintf(stdout, "Message- %s\n", resp->message);
    }

    send(resp->client_fd, HTTP200, strlen(HTTP200) + 1, 0);
    
    char* message = get_message(resp->message);

    if(debug)
        fprintf(stdout, "Message:\n%s\n", message);

    int num_chars = strlen(message);
    int num_words = count_words(message);
    int num_lines = count_lines(message);

    char* response = (char*)malloc(100);
    sprintf(response, "Number of characters: %d\n", num_chars);
    sprintf(response, "%sNumber of words: %d\n", response, num_words);
    sprintf(response, "%sNumber of lines: %d\n", response, num_lines);

    char* content_length = (char*)malloc(100);
    sprintf(content_length, "Content-Length: %ld\n\r", strlen(response));

    char* content_type = (char*)malloc(100);
    sprintf(content_type, "Content-Type: text/plain\n\r");

    send(resp->client_fd, content_length, strlen(content_length), 0);
    send(resp->client_fd, content_type, strlen(content_type), 0);
    send(resp->client_fd, "\n", 1, 0);
    send(resp->client_fd, response, strlen(response), 0);

    close(resp->client_fd);
}

void send_get_response(int status, struct get_response* resp){
    if(status == 200) {
        send(resp->client_fd, HTTP200, strlen(HTTP200) + 1, 0);
        if(debug) {
            fprintf(stdout, "GET Request\n");
            fprintf(stdout, "Requested Resource- %s\n", resp->request_path);
            fprintf(stdout, "Response- %s\n", HTTP200);
        }
    }
    else if(status == 404){
        send(resp->client_fd, HTTP404, strlen(HTTP404), 0);
        if(debug) {
            fprintf(stdout, "Requested Resource- %s\n", resp->request_path);
            fprintf(stdout, "Response- %s\n", HTTP404);
        }
    }

    char* full_path = malloc(strlen(resp->root_dir) + strlen(resp->request_path) + 1);
    strcpy(full_path, resp->root_dir);
    strcat(full_path, resp->request_path);
    resp->full_path = full_path;

    int file_fd = open(resp->full_path, O_RDONLY);
    int file_size = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);
    
    char* file_buffer = malloc(file_size * sizeof(char));

    read(file_fd, file_buffer, file_size);

    char *content_length = (char*)malloc(100);
    sprintf(content_length, "Content-Length: %d\n\r", file_size);

    char *content_type = (char*)malloc(100);
    sprintf(content_type, "Content-Type: %s\n\r", get_mime_type(resp->request_path));

    send(resp->client_fd, content_length, strlen(content_length), 0);
    send(resp->client_fd, content_type, strlen(content_type), 0);
    send(resp->client_fd, "\n", 1, 0);
    send(resp->client_fd, file_buffer, file_size, 0);

    close(resp->client_fd);
}

char* get_message(char* message) {
    int start, end;
    
    for(int i = 0; i < strlen(message); i++) {
        if(message[i] == '%' && message[i + 1] == '*' && message[i + 2] == '*' && message[i + 3] == '%') {
            start = i + 4;
            break;
        }
    }

    for(int i = strlen(message) - 1; i >= 3; i--) {
        if(message[i] == '%' && message[i - 1] == '*' && message[i - 2] == '*' && message[i - 3] == '%') {
            end = i - 3;
            break;
        }
    }

    char* msg = (char*)malloc(end - start + 1);
    strncpy(msg, message + start, end - start);
    msg[end - start] = '\0';

    return msg;
}

int count_words(char* message) {
    int count = 0;
    int in_word = 0;

    for(int i=0; i<strlen(message); i++) {
        if(message[i] == ' ' || message[i] == '\n' || message[i] == '\t' || message[i] == '\r') {
            if(in_word == 1) {
                count++;
                in_word = 0;
            }
        }
        else
            in_word = 1;
    }

    if (in_word)
        count++;

    return count;
}

int count_lines(char* message) {
    int count = 1;

    for(int i = 0; i < strlen(message); i++) {
        if(i == 0 && message[i] == '\n')
            count++;
        else if(i > 0 && message[i] == '\n' && message[i - 1] != '\n')
            count++;
    }
    
    return count;
}