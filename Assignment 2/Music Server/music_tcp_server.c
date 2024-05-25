// Implement a TCP server compatible with the TCP client that streams and plays mp3 music as demonstrated in class.

// The music streaming server should take three command line arguments: the port at which it will listen to (P), the root directory of the mp3 song folder (DIR) and the maximum number of streams that it can simultaneously support (N). Note that the same host can request multiple streams by running multiple instances of the client and they should be counted as independent streams.

// The server’s console should display the following information: IP address of the client that has connected along with the song name requested.

// The server must be implemented in a multithreaded way, every request must be handled in a new thread.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <lame/lame.h>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTs 10
char *folder;
#define BUFFER_SIZE2 8192


int capture_audio(snd_pcm_t *handle, char *buffer, int size)
{
    int err;
    printf("In capture audio\n");
    if ((err = snd_pcm_readi(handle, buffer, size)) < 0)
    {
        printf("Error reading from PCM device: %s\n", snd_strerror(err));
        return -1;
    }
    printf("audio read %s\n", buffer);
    return 0;
}
/* relevant code fragments inside main() */
void sender(void* args) {
    printf("In sender\n");
    printf("Folder: %s\n", folder);
    int client_fd = *((int *)args);
    printf("Sending song to client : %d\n", client_fd);

    char buffer1[2]={0};
    if (recv(client_fd, buffer1, sizeof(char)*2, 0) <= 0) 
    {
        perror("Error receiving choice from client");
        close(client_fd);
        return;
    }
    printf("Received choice from client: %s\n", buffer1);

    // Send the requested song to the client
    //if -1 record microphone and stream it to the client
    if (strcmp(buffer1, "-1") == 0) 
    {

            int i;
            int err;
            char *bufer;
            int buffer_frames = 128;
            unsigned int rate = 44100;
            snd_pcm_t *capture_handle;
            snd_pcm_hw_params_t *hw_params;
            snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
            char *mp3_buffer; // Buffer for holding encoded MP3 data
            int mp3_bytes;

            if ((err = snd_pcm_open (&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
                exit (1);
            }

            fprintf(stdout, "audio interface opened\n");

            if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
                fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                        snd_strerror (err));
                exit (1);
            }

            fprintf(stdout, "hw_params allocated\n");

            if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
                fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                        snd_strerror (err));
                exit (1);
            }

            fprintf(stdout, "hw_params initialized\n");

            if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
                fprintf (stderr, "cannot set access type (%s)\n",
                        snd_strerror (err));
                exit (1);
            }

            fprintf(stdout, "hw_params access setted\n");

            if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
                fprintf (stderr, "cannot set sample format (%s)\n",
                        snd_strerror (err));
                exit (1);
            }

            fprintf(stdout, "hw_params format setted\n");

            if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
                fprintf (stderr, "cannot set sample rate (%s)\n",
                        snd_strerror (err));
                exit (1);
            }

            fprintf(stdout, "hw_params rate setted\n");

            if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
                fprintf (stderr, "cannot set channel count (%s)\n",
                        snd_strerror (err));
                exit (1);
            }

            fprintf(stdout, "hw_params channels setted\n");

            if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
                fprintf (stderr, "cannot set parameters (%s)\n",
                        snd_strerror (err));
                exit (1);
            }

            fprintf(stdout, "hw_params setted\n");

            snd_pcm_hw_params_free (hw_params);

            fprintf(stdout, "hw_params freed\n");

            if ((err = snd_pcm_prepare (capture_handle)) < 0) {
                fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                        snd_strerror (err));
                exit (1);
            }

            fprintf(stdout, "audio interface prepared\n");

            bufer = malloc(128 * snd_pcm_format_width(format) / 8 * 2);

            fprintf(stdout, "buffer allocated\n");
            // lame initialization
            lame_global_flags *lame_flags = lame_init();
            lame_set_in_samplerate(lame_flags, rate);
            lame_set_num_channels(lame_flags, 1); // Adjust the number of channels if needed
            lame_set_out_samplerate(lame_flags, rate);
            lame_set_brate(lame_flags, 128);
            for (i = 0; i < 1000; ++i) {
                if ((err = snd_pcm_readi (capture_handle, bufer, buffer_frames)) != buffer_frames)
                {
                    fprintf (stderr, "read from audio interface failed (%s)\n", snd_strerror (err));
                    exit (1);
                }
                //here buffer has the raw bytes in 
                //    channels: 2
                //   sample rate: 44100
                //   sample size: 16 bits
                //   endian: little
                //   compression: raw
                //   audio: signed integer
                //   bitwidth: 16 bits
                //   buffer_frames: 128
                // we convert it to mp3 using lame
                mp3_bytes = lame_encode_buffer_interleaved_ieee_float(lame_flags, bufer, buffer_frames, mp3_buffer, BUFFER_SIZE2);
                // Send the encoded MP3 data to the client
                //
                //printf("Sending %d bytes of MP3 data\n", mp3_bytes);
                if (send(client_fd, mp3_buffer, mp3_bytes, 0) < 0) {
                    perror("Error sending data to client");
                    break;
                }
            }

            fprintf(stderr, "%s", bufer); //Sir

            free(bufer);

            fprintf(stdout, "buffer freed\n");

            snd_pcm_close (capture_handle);
    }
    else 
    {

        char song_path[100];
        strcpy(song_path, folder);
        strncat(song_path, buffer1,1);
        strcat(song_path, ".");
        strcat(song_path, "mp3");
        printf("Song path: %s here", song_path);
        FILE *mp3 = fopen(song_path, "rb");
        if (!mp3) 
        {
            perror("Error opening mp3 file");
            close(client_fd);
            return;
        }
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), mp3)) > 0)
        {
            ssize_t bytes_sent = send(client_fd, buffer, bytes_read, 0);

            if (bytes_sent < 0) 
            {
                perror("Error sending data to client");
                break; // Exit the loop on send error
            } 
            else if (bytes_sent < bytes_read)
            {
                printf("Client closed the connection prematurely\n");
                break; // Exit the loop if the client closed the connection
            }
        }
        if (ferror(mp3))
        {
            perror("Error reading from mp3 file");
        }
        close(client_fd);
        fclose(mp3);
    }

    //end the thread
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int max_clients;
    int port;
    //receive folder as command line argument
    if(argc<3)
    {
        printf("Usage: %s <port> <max_clients> <folder>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);
    max_clients=atoi(argv[2]);
    folder = argv[3];
    folder = strcat(folder, "/");
    printf("Folder: %s\n", folder);
    // SOCKET - Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address details
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // BIND - Bind socket to address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) 
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // LISTEN - Start listening for incoming connections
    if (listen(server_fd, max_clients) < 0)
    {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while(1) 
    {
	    // ACCEPT - accept incoming connection
	    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("Acceptance failed");
            exit(EXIT_FAILURE);
	    }

        //put onto a thread
	    char client_ip[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
	    printf("Client IP address: %s\n", client_ip);
        pthread_t fun_a_thread;
        pthread_create(&fun_a_thread, NULL, (void *)&sender, &client_fd);
	    //close(client_fd);
    }
    printf("Server stopped\n");
    
    close(server_fd);
    return 0;
}