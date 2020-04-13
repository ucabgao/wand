// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <socket.h> 
// #include <stdlib.h> 
#include <string.h>
// #include "hello.h"
int x() {
	printf("test");
	return 1;
}
int main() 
{ 
	// int sfd = socket(AF_INET, SOCK_STREAM,0);
	dup2(sfd,0);
	dup2(sfd,1); // can I tell what the two arguments are? I want to know that the firs argument is a socketfd and the second argument is 0, 1 or 2
	dup2(sfd,2);
	x();
	return 0;
} 
