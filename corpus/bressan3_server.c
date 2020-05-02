#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <socket.h>

// Message Size
#define BUFFER_SIZE 1024
#define ADDRESS "127.0.0.1"
#define PORT 8888

int main(int argc , char **argv)
{
    int serverSocket, clientSocket, pid;
    char temp[256];
    char path[256];
    struct sockaddr_in server, client;
    
    // Creates socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (serverSocket == -1) {
        printf("Error. Could not create socket.\n");
        exit(-1);
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ADDRESS);
    server.sin_port = htons(PORT);
    
    bind(serverSocket, (struct sockaddr *)&server, sizeof(server));
    
    // Listen for new connections
    listen(serverSocket , 1);
    printf("Listening on Port %d...Waiting for incoming connections...\n", PORT);
    
    FILE *stream = fdopen(dup(STDERR_FILENO), "w");
    setbuf(stream, NULL);
    
    char clientMessage[BUFFER_SIZE];
    memset(clientMessage, 0, sizeof(clientMessage));
    
    socklen_t clientSize = sizeof(client);
    
    //Ensures that the start path is ~/
    if(chdir(getenv("HOME")) == 0) {
        getcwd(temp, sizeof(temp));
    }
    
    while ((clientSocket = accept(serverSocket, (struct sockaddr*)&client, &clientSize))) {
        printf("Client connected.\n");
        if ((pid = fork())) printf("Child PID = %d\n", pid);
        else {
            pid = getpid();
            dup2(clientSocket, 2);
            while (true){
                ssize_t readc = 0;
                int filled = 0;
                while (true) {
                    readc = recv(clientSocket, clientMessage, BUFFER_SIZE-filled-1, 0);
                    if (!readc) break;
                    filled += readc;
                    // Finished receiving message
                    if (clientMessage[filled - 1] == '\0') break;
                }
                if (!readc)
                    break;
                // Since cd is not a program, it won't execute by simply doing system(cd [path])
                if ((clientMessage[0] == 'c' || clientMessage[0] == 'C') && (clientMessage[1] == 'd' || clientMessage[1] == 'D')) {
                    strcpy(path, &clientMessage[3]);
                    path[strlen(path) - 1] = '\0';
                    if(chdir(path) == 0) {
                        getcwd(temp, sizeof(temp));
                    } else {
                        printf("Path %s not found!\n", path);
                    }
                }
                else {
                    system(clientMessage);
                }
                send(clientSocket, "> ", 3, (intptr_t)signal(SIGPIPE, SIG_IGN));
            }
            close(clientSocket);
            exit(0);
        }
    }
    
    fclose(stream);
    close(serverSocket);
    
    return 0;
}

