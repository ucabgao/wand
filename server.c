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
	int server_fd2;
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
	// server_fd2 = socket(AF_INET, SOCK_STREAM, 0);
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

	// if (listen(server_fd, 3) == 0) {
	// 	printf("Test\n");
	// 	if(0)
	// 		bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	// } else if (bind(server_fd, (struct sockaddr *)&address, sizeof(address) == -1)) {
	// 	listen(server_fd, 3);
	// } else {
	// 	new_socket = accept(server_fd, (struct sockaddr *)&address, 
	// 				(socklen_t*)&addrlen);
	// } //check if i got no change of state inside, what happens? this works


	int i = 0;

	for(i; i==1 ; i++) {
		for(int j = 1; j < 1; j++) {
			bind(server_fd, (struct sockaddr *)&address, sizeof(address));
			i++;
			printf("1Val of i: %d\n",i);
		}
		listen(server_fd, 3);
		// new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
		i++;
		printf("2Val of i: %d\n",i);
	} // i>bX, b>bX, b>lX, i>lX, l>bX, l>lX, i>cX, l>cX

// do {
// 	bind(server_fd, (struct sockaddr *)&address, sizeof(address));
// 	do {
// 		listen(server_fd, 3);
// 		i++;
// 	} while (i < 2);
// 	i++;
// } while (i < 4);

	// if(1) {
	// 	if (0) {
	// 		i++;
	// 		bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	// 	} else {
	// 		i++;
	// 		listen(server_fd, 3);
	// 	}
	// } else if (0) {
	// 	i++;
	// 	listen(server_fd, 3);

	// 	if(1) {
	// 		i++;
	// 		new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	// 	}
	// } else {
	// 	i++;
	// 	new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	// }


// src = I
		// while (i < 2) {
		// while(i != 0) {
		// while(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
		// while(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1 && listen(server_fd, 3) == -1) { // src = B
		// 	bind(server_fd, (struct sockaddr *)&address, sizeof(address));
		// 	bind(server_fd2, (struct sockaddr *)&address, sizeof(address));
			// listen(server_fd2, 2);
	 // // 		while (listen(server_fd, 3) != -1) { // src = L
	 // // 			i++;
	 // // 			new_socket = accept(server_fd, (struct sockaddr *)&address, 
		// // 			(socklen_t*)&addrlen);
	 // // 		}
	 // 		// src = L
	 	// 	while (0) {
	 	// 		i++;
	 			// listen(server_fd, 3);
	 	// 		new_socket = accept(server_fd, (struct sockaddr *)&address, 
			// 		(socklen_t*)&addrlen);
	 		// }
	 		
	 	// } // if there is a socket func in the while cond then that has to be my final src
	 	// if there isn't, then my final src will be the final state at the end of the block
	

	// while ((i++) == 1) {
	// 	listen(server_fd, 2);
	// 	i++;
	// }

	// do {
	// 	bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	// 	do {
	// 		listen(server_fd, 3);
	// 		while (i == 1) {
	// 			new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	// 			// i++;
	// 		}
	// 	} while (0);
	// } while (0);
	// i>b, b>l, l>l, l>b, l>o, o>o, o>l, o>b, l>c, o>c

	printf("Val of i: %d\n",i);
	// close(server_fd2);

	// if (0) {
	// 	while(0) {
	// 		bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	// 		listen(server_fd, 3);
	// 	}
	// }



	// if (1) {
	// 	bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	// } else {
	// 	if (0) // todo: if function in condition -> always "executed" -> change state
	// 		listen(server_fd, 3);
	// 	else
	// 		new_socket = accept(server_fd, (struct sockaddr *)&address, 
	// 				(socklen_t*)&addrlen);
	// } //check if i got no change of state inside, what happens? this works


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
