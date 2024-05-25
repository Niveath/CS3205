#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>

extern int max_username_length;
extern int max_message_length;
extern int max_message_buffer;

struct setup_args {
    int max_clients;
    int timeout;
};

struct client_args {
    int client_fd;
    char* username;
    struct client_args* next;
};

struct client_message {
    int sender_fd;
    char* sender_username;
    char* message;
    int length;
    struct client_message* next;
};

struct timeout_args {
    int client_fd;
    int is_timedout;
    time_t last_message;
    pthread_mutex_t timeout_mutex;
};

void* setup(void* args);

void* handle_client(void* args);

void* broadcast_messages(void* args);

void* check_timeout(void* args);

char* get_current_users(int client_fd);