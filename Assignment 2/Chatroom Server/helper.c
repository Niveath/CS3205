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