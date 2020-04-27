// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <socket.h> 
#include <stdlib.h> 
#include <string.h>
#include "hello.h"
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
	// int sfd[3];
	int *fd;
	// fd = NULL;
	*fd = socket(AF_INET, SOCK_STREAM,0);

	if (bind(*fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}

	// sfd[0] = socket(AF_INET, SOCK_STREAM,0);
	// dup2(sfd,0);
	// dup2(sfd,1); // can I tell what the two arguments are? I want to know that the firs argument is a socketfd and the second argument is 0, 1 or 2
	// dup2(sfd,2);
	// do{
	// x();
	// } while(1);
	// if (0)
	// 	b();
	// else
	// 	a();
	return 0;
} 
