Chat Server

Compiling the server:
	gcc serverQ3_cs21b060_cs21b069.c client_thread.c helper.c -o chat_server -pthread

Running the server:
	./chat_server <port_number> <max_users> <timeout duration>

Description of working of server:
	- The server binds to the given port on the device
	- For every new connection a new thread is created.
	- For every new connection, it checks if there is space in the server. If no, it sends a error message to this client and disconnects it.
	- If there is space in the server, it checks if the username chosen by the user is unique. If no, it sends a error message to the client and disconnects it.
	- There are two specials therads, a global broadcast thread and a per-user timeout thread. The broadcast thread checks a global linked list for any new messages sent by users and broadcasts the message to other users. The per-user timeout thread keeps checking if the user has remained inactive for too long and if true, sends a inactivity message to that user and disconnects it.
	- The set of active clients are maintained using a global linked list and the list of messages sent by users that are yet to be broadcasted to other users is also maintained in a global linked list. Each of these linked lists have associated locks to prevent data-race conditions.