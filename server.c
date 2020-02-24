// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <socket.h> 
#include <stdlib.h> 
#include <string.h>
#include "hello.h"
#define PORT 8080 
int main() 
{ 
	int server_fd;
	int new_socket;
	int valread; 
	struct sockaddr_in address; // sockaddr_in is specific for inet
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char *hello = "Hello from server"; 
	printf("Testing\n");
	printhelloworld();

	// Creating socket file descriptor
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
												&opt, sizeof(opt))) // if successful, return 0 -> condition is false
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	
	// Forcefully attaching socket to the port 8080 

	if (1) {
		bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	} else {
		if (0) // todo: if function in condition -> always "executed" -> change state
			listen(server_fd, 3);
		else
			new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen);
	}

	// if (1) {
	// 	bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	// } else 
	// if (1) {
	// 	listen(server_fd, 3);
	// } else if (1) {
	// 	if (1) {
	// 		listen(server_fd, 3);
	// 	} else {
	// 		listen(server_fd, 3);
	// 		new_socket = accept(server_fd, (struct sockaddr *)&address, 
	// 				(socklen_t*)&addrlen);
	// 	}
	// }

	// if (bind(server_fd, (struct sockaddr *)&address, 
	// 							sizeof(address))<0) 
	// { 
	// 	perror("bind failed"); 
	// 	exit(EXIT_FAILURE); 
	// } 
	// if (listen(server_fd, 3) < 0) 
	// { 
	// 	perror("listen"); 
	// 	exit(EXIT_FAILURE); 
	// } 
	// new_socket = accept(server_fd, (struct sockaddr *)&address, 
	// 				(socklen_t*)&addrlen);
	// if (new_socket<0) 
	// { 
	// 	perror("accept"); 
	// 	exit(EXIT_FAILURE); 
	// } 
	// valread = read( new_socket , buffer, 1024); 
	// printf("%s\n",buffer ); 
	// send(new_socket , hello , strlen(hello) , 0 );

	close(server_fd);
	printf("Hello message sent\n"); 
	return 0; 
} 
