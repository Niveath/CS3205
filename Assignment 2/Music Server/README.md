Music TCP Server :

Usage : 
Compile as follows :
gcc serverQ1_cs21b060_cs21b069.c  -o serverQ1_cs21b060_cs21b069 -lasound -lmp3lame

Execute as Follows :
./serverQ1_cs21b060_cs21b069 <port> <max_clients> <folder>

The programs runs a server which implements a multi thread version of a tcp streaming server


The server initializes by creating a TCP socket and binding it to a specified port.
It sets up the server address details and starts listening for incoming connections.


When a client connection is accepted, the server spawns a new thread to handle the client request.
Each client request is processed independently, allowing concurrent streaming to multiple clients.

Clients can send requests to the server to stream music from two sources:
Song Request: Clients can request songs from a specified directory by sending the index of the desired song.
Microphone Stream: Clients can choose to stream audio from a microphone connected to the server by sending a specific code.

If a client requests a song, the server locates the corresponding MP3 file in the specified directory and streams it to the client.
If a client requests a microphone stream, the server captures audio from the microphone using ALSA which generates a pcm file which subsequently is converted to MP3 stream using the LAME Package and streamed to the client, encodes it in real-time to MP3 format, and streams it to the client.

Note :
=> The implementation for handling clients as N peers is done using a global counter 