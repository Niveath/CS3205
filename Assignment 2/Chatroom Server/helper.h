#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern int debug;

int create_socket(int domain, int type, int protocol);

struct sockaddr_in bind_socket(int family, int addr, int port, int sock_fd);