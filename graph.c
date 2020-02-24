#include <sys/socket.h>
#include "interpreter.h"


/* structure of a socket state graph node*/
struct SocketStateGraph
{
    enum SocketState State;         /* state of the socket */
    short int Line;                  /* line number of the function call */
    struct SocketStateGraph *Next;  /* next (actually previous) node in the graph) */
};

struct SocketNFA
{
    enum SocketState SourceState;         /* state of the socket */
    enum SocketState DestState;         /* state of the socket */
    short int Line;                  /* line number of the function call */
    struct SocketNFA *Next;  /* next (actually previous) node in the graph) */
};


struct Source
{
    enum SocketState State;         /* state of the socket */
    struct Source *Next;
};


void SocketAddIgnoreLevel(Picoc *pc) {
    struct IgnoreLevel *newIgnoreLevel = (struct IgnoreLevel *) malloc(sizeof(struct IgnoreLevel));

    newIgnoreLevel->Level = pc->level;
    newIgnoreLevel->Next = pc->SourceIgnoreLevel;

    pc->SourceIgnoreLevel = newIgnoreLevel;
}


void SocketRemoveIgnoreLevel(Picoc *pc) {
    struct IgnoreLevel *head = pc->SourceIgnoreLevel, *prev;

    if (head != NULL && head->Level == pc->level) 
    { 
        pc->SourceIgnoreLevel = head->Next;
        // free(temp);               // free old head 
        return; 
    } 
  
    while (head != NULL && head->Level == pc->level) 
    { 
        prev = head; 
        head = head->Next; 
    } 
  
    // If key was not present in linked list 
    if (head == NULL) return; 
  
    // Unlink the node from linked list 
    prev->Next = head->Next;
}

int SocketCheckIgnoreLevel(Picoc *pc) {
    struct IgnoreLevel *head = pc->SourceIgnoreLevel;
    int level = pc->level;
    // struct IgnoreLevel *newIgnoreLevel = (struct IgnoreLevel *) malloc(sizeof(struct IgnoreLevel));

    while(head != NULL) {
        if (head->Level == level) {
            return 0;
        }

        head = head->Next;
    }

    return 1;
}

// struct SourceStack
// {
//     enum SocketState State;
//     struct SourceStack *Next;
// };



void SocketCopy(Picoc *pc, struct Socket *newSocketList) {
    // memcpy((void *)newSocketList, (void *)pc->SocketList, sizeof(*newSocketList));

    struct Socket *head = pc->SocketList;
    struct Socket *nSL = NULL;

    while (head != NULL) {
        struct Socket *newSocket = (struct Socket *) malloc(sizeof(struct Socket));
        newSocket->FileDescriptor = head->FileDescriptor;
        newSocket->SourceStack = NULL;
        newSocket->Next = nSL;
        struct Source *sourceHead = head->SourceStack;

        while (sourceHead != NULL) {
            struct Source *newSource = (struct Source *) malloc(sizeof(struct Source));
            newSource->State = sourceHead->State;
            newSource->Next = newSocket->SourceStack;

            newSocket->SourceStack = newSource;

            sourceHead = sourceHead->Next;
        }

        nSL = newSocket;
        
        head = head->Next;
    }
    if (nSL != NULL)
        *newSocketList = *nSL;
}

/*  */
void SocketInit(Picoc *pc)
{
    // struct Socket *newSocket = (struct Socket *) malloc(sizeof(struct Socket));
    // newSocket->FileDescriptor = -1;
    // newSocket->Next = NULL;

    pc->SocketList = NULL;
    pc->level = 0;
    pc->PreviousToken = TokenNone;
}

void AddSocket(Picoc *pc, int fd, int type, short int line) {
    struct Socket *head = pc->SocketList;
    struct Socket *newSocket = (struct Socket *) malloc(sizeof(struct Socket));
    struct Source *newSource = (struct Source *) malloc(sizeof(struct Source));
    struct SocketStateGraph *newSocketStateGraph = (struct SocketStateGraph *) malloc(sizeof(struct SocketStateGraph));

    newSocketStateGraph->State = Creation;
    newSocketStateGraph->Line = line;
    newSocketStateGraph->Next = NULL;

    newSource->State = Creation;
    newSource->Next = NULL;

    newSocket->FileDescriptor = fd;
    newSocket->Type = type;
    newSocket->StateGraph = newSocketStateGraph;
    newSocket->NFA = NULL;
    // newSocket->Source = newSource;
    newSocket->SourceStack = newSource;
    newSocket->Next = head;

    pc->SocketList = newSocket;
}

// void {
//     while( != NULL) { //socket
//         while ( != NULL) { //source

//         }
//     }
// }

struct Socket *FindSocket(struct Socket *s, int fd) {
    while(s != NULL) {
        if (s->FileDescriptor == fd) {

            return s;
        }
        s = s->Next;
    }

    return NULL;
}

enum SocketState FindState(const char *FuncName) {
    enum SocketState s = -1;

    if (strcmp(FuncName, "bind") == 0) {
        s = Binding;
    } else if (strcmp(FuncName, "listen") == 0) {
        s = Listening;
    } else if (strcmp(FuncName, "accept") == 0) {
        s = Open;
    } else if (strcmp(FuncName, "close") == 0) {
        s = Close;
    }

    return s;
}


void AddSocketNFA(Picoc *pc, int fd, short int line, const char *FuncName) {
    struct Socket *head = pc->SocketList;
    struct Socket *s = FindSocket(head, fd);
    // struct SocketNFA *curSocketNFA = s->NFA;
    

    enum SocketState dst = FindState(FuncName);

    struct Source *source = s->SourceStack;

    while (source != NULL) {
        struct SocketNFA *newSocketNFA = (struct SocketNFA *) malloc(sizeof(struct SocketNFA));
        newSocketNFA->SourceState = source->State;
        newSocketNFA->DestState = dst;
        newSocketNFA->Line = line;
        newSocketNFA->Next = s->NFA;

        s->NFA = newSocketNFA;

        source = source->Next;
    }
    // newSocketNFA->SourceState = s->Source;
    // newSocketNFA->DestState = dst;
    // newSocketNFA->Line = line;
    // newSocketNFA->Next = curSocketNFA;

    // s->Source = dst;
}

// void AddSocketBranch() {

// }

void UpdateSource(Picoc *pc, int fd, const char *FuncName) {
    struct Socket *head = pc->SocketList; 
    struct Socket *socket = FindSocket(head, fd);
    // struct Source *source = socket->SourceStack;
    struct Source *newSource = (struct Source *) malloc(sizeof(struct Source));
    
    newSource->State = FindState(FuncName);
    newSource->Next = NULL;

    socket->SourceStack = newSource;
    // while (source != NULL) {
    //     source->FindState(FuncName);
    // }

}

void AddSocketStateGraph(Picoc *pc, int fd, short int line, const char *FuncName) {
    struct Socket *head = pc->SocketList;
    struct Socket *s = FindSocket(head, fd);
    struct SocketStateGraph *curSocketStateGraph = s->StateGraph;
    struct SocketStateGraph *newSocketStateGraph = (struct SocketStateGraph *) malloc(sizeof(struct SocketStateGraph));

    if (strcmp(FuncName, "bind") == 0) {
        if (s->Type == SOCK_STREAM) {
            newSocketStateGraph->State = Binding;
        } else {
            newSocketStateGraph->State = Open;
        }
    } else if (strcmp(FuncName, "listen") == 0) {
        newSocketStateGraph->State = Listening;
    } else if (strcmp(FuncName, "accept") == 0) {
        newSocketStateGraph->State = Open;
    } else if (strcmp(FuncName, "close") == 0) {
        newSocketStateGraph->State = Close;
    }

    newSocketStateGraph->Line = line;
    newSocketStateGraph->Next = curSocketStateGraph;

    s->StateGraph = newSocketStateGraph;
}

void DisplaySocket(Picoc *pc) {
    struct Socket *head = pc->SocketList;

    while (head != NULL) {
        printf("Socket FD %d - ", head->FileDescriptor);
        if (head->Type == SOCK_STREAM) {
            printf("TCP: ");
        } else if (head->Type == SOCK_DGRAM) {
            printf("UDP: ");
        }

        struct SocketStateGraph *headSSG = head->StateGraph;

        while (headSSG != NULL) {
            char stateStr[20];
            switch (headSSG->State) {
                case 0: strcpy(stateStr, "Creation"); break;
                case 1: strcpy(stateStr, "Binding"); break;
                case 2: strcpy(stateStr, "Listening"); break;
                case 3: strcpy(stateStr, "Open"); break;
                case 4: strcpy(stateStr, "Close"); break;
            }
            printf("%s (line %d) <- ", stateStr, headSSG->Line);
            headSSG = headSSG->Next;
        }

        printf("\n");
        head = head->Next;
    }
}

char *GetStateNameString(enum SocketState state, char *stateStr) {
    switch (state) {
        case 0: strcpy(stateStr, "Creation"); break;
        case 1: strcpy(stateStr, "Binding"); break;
        case 2: strcpy(stateStr, "Listening"); break;
        case 3: strcpy(stateStr, "Open"); break;
        case 4: strcpy(stateStr, "Close"); break;
    }

    return stateStr;
}

void DisplayNFA(Picoc *pc) {
    struct Socket *head = pc->SocketList;
    char stateStr[20];

    while (head != NULL) {
        printf("Socket FD %d - ", head->FileDescriptor);
        if (head->Type == SOCK_STREAM) {
            printf("TCP - ");
        } else if (head->Type == SOCK_DGRAM) {
            printf("UDP - ");
        }

        printf("NFA: \n");

        struct SocketNFA *headNFA = head->NFA;

        while (headNFA != NULL) {
            printf("Line %d : ", headNFA->Line);
            printf("(%s -> ", GetStateNameString(headNFA->SourceState, stateStr));
            printf("%s)\n", GetStateNameString(headNFA->DestState, stateStr));
            headNFA = headNFA->Next;
        }

        printf("\n");
        head = head->Next;
    }
}

void SocketRevertSource(Picoc *pc, struct Socket *oldSocketList) {
    struct Socket *head = pc->SocketList;
    struct Socket *oldHead = oldSocketList;

    if (pc->SocketList != NULL)
        while (oldHead != NULL) {
            int fd = oldHead->FileDescriptor;
            struct Socket *s = FindSocket(head, fd);

            s->SourceStack = oldHead->SourceStack;

            oldHead = oldHead->Next;
        }
}

void SocketCombineSource(Picoc *pc, struct Socket *oldSocketList) {
    struct Socket *head = pc->SocketList;
    struct Socket *oldHead = oldSocketList;

    while(head != NULL) {
        while(oldHead != NULL) {
            if (head->FileDescriptor == oldHead->FileDescriptor) {
                struct Source *sourceHead = head->SourceStack;

                while(sourceHead->Next != NULL) {
                    sourceHead = sourceHead->Next;
                }

                sourceHead->Next = oldHead->SourceStack;

                struct Source *sourceHead2, *dup; 
                sourceHead = head->SourceStack;
                while (sourceHead != NULL && sourceHead->Next != NULL) 
                { 
                    sourceHead2 = sourceHead; 
              
                    /* Compare the picked element with rest 
                       of the elements */
                    while (sourceHead2->Next != NULL) 
                    { 
                        /* If duplicate then delete it */
                        if (sourceHead->State == sourceHead2->Next->State) 
                        { 
                            /* sequence of steps is important here */
                            dup = sourceHead2->Next; 
                            sourceHead2->Next = sourceHead2->Next->Next; 
                            // delete(dup); 
                        } 
                        else /* This is tricky */
                            sourceHead2 = sourceHead2->Next; 
                    } 
                    sourceHead = sourceHead->Next; 
                } 
                break;
            }

            oldHead = oldHead->Next;
        }


        head = head->Next;
    }
}
