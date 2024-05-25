#include "helper.h"

int create_socket(int domain, int type, int protocol) {
    int socket_fd = socket(domain, type, protocol);
    if(socket_fd == 0) {
        fprintf(stderr, "Socket creation failed :(\n");
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

struct sockaddr_in bind_socket(int family, int addr, int port, int sock_fd) {
    struct sockaddr_in address;
    address.sin_family = family;
    address.sin_addr.s_addr = addr;
    address.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        fprintf(stderr, "Socket binding failed :(\n");
        exit(EXIT_FAILURE);
    }

    return address;
}

int path_exists(char* path) {
    if(access(path, F_OK) == -1) return 0;
    return 1;
}

char* get_mime_type(char* file_path) {
    char* ext = strrchr(file_path, '.');

    if (ext != NULL) {
        if (strcmp(ext, ".html") == 0)
            return "text/html";
        else if (strcmp(ext, ".css") == 0)
            return "text/css";
        else if (strcmp(ext, ".js") == 0)
            return "text/javascript";
        else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
            return "image/jpeg";
        else if (strcmp(ext, ".png") == 0)
            return "image/png";
    }

    return "application/octet-stream"; 
}