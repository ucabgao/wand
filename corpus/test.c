// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <socket.h> 
#include <stdlib.h> 
#include <string.h>
#define PORT 8080
// int x() {
// 	printf("test");
// 	return 1;
// }

// void c() {
// 	printhelloworld();
// 	fork();	
// }

// void b() {
// 	c();
// }

// void a() {
// 	b();
// }

int main() 
{ 
	struct sockaddr_in address; // sockaddr_in is specific for inet
	int addrlen = sizeof(address); 
	int parentSocket;
	int clientSocket[3];
	
	parentSocket = socket(AF_INET, SOCK_STREAM,0);

	if (parentSocket == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(parentSocket, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

	if (listen(parentSocket, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 

	for (int i = 0; i < 3; i++) {
		clientSocket[i] = accept(parentSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
	}

	return 0;
} 
