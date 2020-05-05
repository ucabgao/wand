#include <unistd.h> 
#include <stdio.h> 
#include <socket.h> 
#include <stdlib.h> 
#include <string.h>
#define PORT 8080

int main() 
{ 
	int socketfd;
	socketfd = socket(AF_INET, SOCK_STREAM,0);
	return 0;
} 
