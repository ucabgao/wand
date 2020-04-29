#include <unistd.h> 
#include <stdio.h> 
#include <socket.h> 
#include <stdlib.h> 
#include <string.h>
#define PORT 8080

int main() 
{ 
	struct sockaddr_in address; // sockaddr_in is specific for inet
	int addrlen = sizeof(address); 
	int parentSocket;
	int clientSocket[3];
	int valread; 
	char buffer[1024] = {0}; 
	char *hello = "Hello from server"; 

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
		if (clientSocket[i]<0) 
		{ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 
		valread = read( clientSocket[i] , buffer, 1024); 
		printf("%s\n",buffer ); 
		send(clientSocket[i] , hello , strlen(hello) , 0 );
	}

	return 0;
} 
