#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>

extern char* HTTP200;
extern char*  HTTP404;
#define BUFFERSIZE 1024

struct get_response {
    int client_fd;
    char* root_dir;
    char* request_path;
    char* full_path;
};

struct post_response {
    int client_fd;
    char* message;
};

void* get_response(void* args);

void* post_response(void* args);

void send_get_response(int status, struct get_response* resp);

char* get_message(char* message);

int count_words(char* message);
int count_lines(char* message);