HTTP Server:

Compiling the server:
	gcc serverQ2_cs21b060_cs21b069.c helper.c response.c -o http_server -pthread

Running the server
	./http_server <port_number> <root_file_directory>

Description of working of the server:
	- This HTTP server does not use persistent connections. It sets up a new TCP connection for every request.
	- Each request is handled in a separate thread. 
	- The thread checks if the request is a GET or a POST request and appropriately handles these requests/
	- If it is GET request, we first check if the request resource exists and if yes, we send it, else we send a 404 error and serve the 404.html page
	- If it is a POST request, we get the number of characters, words and sentences in the received message and return it to the client.