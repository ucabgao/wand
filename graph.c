#include <sys/socket.h>
#include "interpreter.h"
#include "cjson/cJSON.h"

const char * INTERESTEDFUNC[] = { "socket", "bind", "listen", "accept", "close", "read", "recv", "recvfrom", "write", "send", "sendto", "dup2", "fork", "execl", "execlp", "execle", "execv", "execvp", "execvpe"};

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

    newIgnoreLevel->Level = pc->Level;
    newIgnoreLevel->Next = pc->SourceIgnoreLevel;

    pc->SourceIgnoreLevel = newIgnoreLevel;
}


void SocketRemoveIgnoreLevel(Picoc *pc) {
    struct IgnoreLevel *head = pc->SourceIgnoreLevel, *prev;

    if (head != NULL && head->Level == pc->Level) 
    { 
        pc->SourceIgnoreLevel = head->Next;
        // free(temp);               // free old head 
        return; 
    } 
  
    while (head != NULL && head->Level == pc->Level) 
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
    int level = pc->Level;
    // struct IgnoreLevel *newIgnoreLevel = (struct IgnoreLevel *) malloc(sizeof(struct IgnoreLevel));

    while(head != NULL) {
        if (head->Level == level) {
            return FALSE;
        }

        head = head->Next;
    }

    return TRUE;
}

void SocketCopy(Picoc *pc, struct Socket *newSocketList) {
    // memcpy((void *)newSocketList, (void *)pc->SocketList, sizeof(*newSocketList));

    struct Socket *head = pc->SocketList;
    struct Socket *nSL = NULL;

    while (head != NULL) {
        struct Socket *newSocket = (struct Socket *) malloc(sizeof(struct Socket));
        newSocket->FileDescriptor = head->FileDescriptor;
        newSocket->Identifier = head->Identifier;
        newSocket->CurrentState = head->CurrentState;
        newSocket->SourceStack = NULL;
        newSocket->ParentIdentifier = head->ParentIdentifier;
        newSocket->Line = head->Line;
        newSocket->Dup2Arr = head->Dup2Arr;
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
    pc->Level = 0;
    pc->PreviousToken = TokenNone;
    pc->IdentifierAssignedTo = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
    pc->VarIdList = NULL;
    pc->FuncIdList = NULL;
    pc->CharacteristicList = NULL;
}

// void AddSocket(Picoc *pc, int fd, int type, short int line, int parent) {
void AddSocket(Picoc *pc, char *identifier, char *sockettype, short int line, char* parent) {
    struct Socket *head = pc->SocketList;
    struct Socket *newSocket = (struct Socket *) malloc(sizeof(struct Socket));
    struct Source *newSource = (struct Source *) malloc(sizeof(struct Source));
    struct SocketStateGraph *newSocketStateGraph = (struct SocketStateGraph *) malloc(sizeof(struct SocketStateGraph));
    struct SocketNFA *newSocketNFA = (struct SocketNFA *) malloc(sizeof(struct SocketNFA));

    newSocketStateGraph->State = NotListening;
    newSocketStateGraph->Line = line;
    newSocketStateGraph->Next = NULL;

    newSource->State = Initial;
    newSource->Next = NULL;


    newSocket->CurrentState = Initial;

    if (strcmp(parent,"") && !strcmp(sockettype,"")) { // parent is not "" and type is ""
        newSocketStateGraph->State = Connected;
        newSource->State = Connected;
        newSocket->CurrentState = NotReadingOrWriting;
    }


    newSocketNFA->SourceState = None;
    newSocketNFA->DestState = newSource->State;
    newSocketNFA->Line = line;
    newSocketNFA->Next = NULL;

    // newSocket->FileDescriptor = fd;
    newSocket->Identifier = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
    strcpy(newSocket->Identifier, identifier);
    newSocket->SocketType = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
    strcpy(newSocket->SocketType, sockettype);
    newSocket->StateGraph = newSocketStateGraph;
    newSocket->NFA = newSocketNFA;
    // newSocket->NFA = NULL;
    // newSocket->Source = newSource;
    newSocket->SourceStack = newSource;

    newSocket->ParentIdentifier = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
    strcpy(newSocket->ParentIdentifier, parent);

    // newSocket->ParentFileDescriptor = parent;
    newSocket->Line = line;
    newSocket->Dup2Arr = (int *) calloc(3, sizeof(int));

    newSocket->Next = head;

    pc->SocketList = newSocket;
}

struct Socket *FindSocket(struct Socket *s, int fd) {
    while(s != NULL) {
        if (s->FileDescriptor == fd) {
        // if (!strcmp(s->Identifier, 0)) {

            return s;
        }
        s = s->Next;
    }

    return NULL;
}

struct Socket *FindSocketByIdentifier(struct Socket *s, char *identifier) {
    while(s != NULL) {
        // if (s->FileDescriptor == fd) {
        if (!strcmp(s->Identifier, identifier)) {

            return s;
        }
        s = s->Next;
    }

    return NULL;
}

int CheckFuncOfInterest(const char *FuncName) {
    // char * fn [] = { "socket", "bind", "listen", "accept", "close", "read", "recv", "recvfrom", "write", "send", "sendto", "dup2", "fork", "execl", "execlp", "execle", "execv", "execvp", "execvpe"};
    int len = sizeof(INTERESTEDFUNC)/sizeof(INTERESTEDFUNC[0]);
    int i;

    for(i = 0; i < len; ++i)
    {
        if(!strcmp(INTERESTEDFUNC[i], FuncName))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int CheckIfReadFunc(const char *FuncName) {
    char * fn [] = { "read", "recv", "recvfrom" };
    int len = sizeof(fn)/sizeof(fn[0]);
    int i;

    for(i = 0; i < len; ++i)
    {
        if(!strcmp(fn[i], FuncName))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int CheckIfWriteFunc(const char *FuncName) {
    char * fn [] = { "write", "send", "sendto" };
    int len = sizeof(fn)/sizeof(fn[0]);
    int i;

    for(i = 0; i < len; ++i)
    {
        if(!strcmp(fn[i], FuncName))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int CheckIfExecFunc(const char *FuncName) {
    char * fn [] = { "execl", "execlp", "execle", "execv", "execvp", "execvpe" };
    int len = sizeof(fn)/sizeof(fn[0]);
    int i;

    for(i = 0; i < len; ++i)
    {
        if(!strcmp(fn[i], FuncName))
        {
            return TRUE;
        }
    }

    return FALSE;
}

enum SocketState FindState(const char *FuncName) {
    enum SocketState s = -1;

    if (!strcmp(FuncName, "bind")) {
        s = Binding;
    } else if (!strcmp(FuncName, "listen") || !strcmp(FuncName, "accept")) {
        s = Listening;
    // } else if (!strcmp(FuncName, "accept")) {
    //     s = AwaitConnection;
    } else if (!strcmp(FuncName, "close")) {
        s = Closed;
    } else if (CheckIfReadFunc(FuncName) /*!strcmp(FuncName, "read")*/) {
        s = Reading;
    } else if (CheckIfWriteFunc(FuncName) /*!strcmp(FuncName, "send")*/) {
        s = Writing;
    }

    return s;
}

enum SocketState StateAbstraction(enum SocketState state) {
    enum SocketState abstractState = -1;

    if (state == Initial) {
        abstractState = NotListening;
    } else if (state == Binding || state == Listening) {
        abstractState = MayBeListening;
    } else if (state == Connected) {
        abstractState = NotReadingOrWriting;
    } else if (state == Reading || state == Writing) {
        abstractState = MayBeReadingOrWriting;
    }

    return abstractState;
}


char *GetStateNameString(enum SocketState state, char *stateStr) {
    switch (state) {
        case 0: strcpy(stateStr, ""); break;
        case 1: strcpy(stateStr, "Initial"); break;
        case 2: strcpy(stateStr, "Binding"); break;
        case 3: strcpy(stateStr, "Listening"); break;
        case 4: strcpy(stateStr, "Closed"); break;
        case 5: strcpy(stateStr, "Connected"); break;
        case 6: strcpy(stateStr, "Reading"); break;
        case 7: strcpy(stateStr, "Writing"); break;
        case 8: strcpy(stateStr, "NotListening"); break;
        case 9: strcpy(stateStr, "MayBeListening"); break;
        case 10: strcpy(stateStr, "NotReadingOrWriting"); break;
        case 11: strcpy(stateStr, "MayBeReadingOrWriting"); break;
    }

    return stateStr;
}

int NFAEdgeExists(struct SocketNFA *socketNFAHead, enum SocketState src, enum SocketState dst, int line) {
    struct SocketNFA *temp = socketNFAHead;

    while(temp != NULL) {
        if (temp->SourceState == src && temp->DestState == dst && temp->Line == line) {
            return TRUE;
        }

        temp = temp->Next;
    }

    return FALSE;
}

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

void AddSocketNFA(Picoc *pc, int fd, short int line, const char *FuncName) {
    struct Socket *head = pc->SocketList;
    struct Socket *s = FindSocket(head, fd);
    // struct SocketNFA *curSocketNFA = s->NFA;
    
    if (s == NULL) {
        return;
    }

    int loop = 0;

    if (CheckIfReadFunc(FuncName) || CheckIfWriteFunc(FuncName)) {
        loop = 1;
    }

    enum SocketState dst = FindState(FuncName);

    struct Source *source = s->SourceStack;

    while (source != NULL) {
        if (!NFAEdgeExists(s->NFA, source->State, dst, line)) {
            struct SocketNFA *newSocketNFA = (struct SocketNFA *) malloc(sizeof(struct SocketNFA));
            newSocketNFA->SourceState = source->State;
            newSocketNFA->DestState = dst;
            newSocketNFA->Line = line;
            newSocketNFA->Next = s->NFA;

            s->NFA = newSocketNFA;
        }

        if (loop && !NFAEdgeExists(s->NFA, dst, source->State, line)) {
            struct SocketNFA *newSocketNFA = (struct SocketNFA *) malloc(sizeof(struct SocketNFA));
            newSocketNFA->SourceState = dst;
            newSocketNFA->DestState = source->State;
            newSocketNFA->Line = line;
            newSocketNFA->Next = s->NFA;

            s->NFA = newSocketNFA;
        }

        source = source->Next;
    }
    // newSocketNFA->SourceState = s->Source;
    // newSocketNFA->DestState = dst;
    // newSocketNFA->Line = line;
    // newSocketNFA->Next = curSocketNFA;

    // s->Source = dst;
    if (!loop)
        UpdateSource(pc, fd, FuncName);
}

// void AddSocketStateGraph(Picoc *pc, int fd, short int line, const char *FuncName) {
void AddSocketStateGraph(Picoc *pc, char *identifier, short int line, const char *FuncName) {
    struct Socket *head = pc->SocketList;
    struct Socket *s = FindSocketByIdentifier(head, identifier);

    if (s) {
        struct SocketStateGraph *curSocketStateGraph = s->StateGraph;
        struct SocketStateGraph *newSocketStateGraph = (struct SocketStateGraph *) malloc(sizeof(struct SocketStateGraph));

        if (!strcmp(FuncName, "bind")) {
            if (!strcmp(s->SocketType, "SOCK_STREAM")) {
                newSocketStateGraph->State = Binding;
            } else {
                newSocketStateGraph->State = Listening;
            }
        } else {
            newSocketStateGraph->State = FindState(FuncName);
        }
        // else if (strcmp(FuncName, "listen") == 0) {
        //     newSocketStateGraph->State = Passive;
        // } else if (strcmp(FuncName, "accept") == 0) {
        //     newSocketStateGraph->State = AwaitConnection;
        // } else if (strcmp(FuncName, "close") == 0) {
        //     newSocketStateGraph->State = Closed;
        // }

        newSocketStateGraph->Line = line;
        newSocketStateGraph->Next = curSocketStateGraph;

        s->StateGraph = newSocketStateGraph;
    }
}

void DisplaySocket(Picoc *pc) {
    struct Socket *head = pc->SocketList;

    while (head != NULL) {
        // printf("Socket FD %d - ", head->FileDescriptor);
        printf("\nSocket ID %s - ", head->Identifier);
        if (!strcmp(head->SocketType, "SOCK_STREAM")) {
            printf("TCP: ");
        } else if (!strcmp(head->SocketType, "SOCK_DGRAM")) {
            printf("UDP: ");
        }

        struct SocketStateGraph *headSSG = head->StateGraph;

        while (headSSG != NULL) {
            char stateStr[20];
            printf("%s (line %d) <- ", GetStateNameString(headSSG->State, stateStr), headSSG->Line);
            headSSG = headSSG->Next;
        }

        char stateStr[20];
        printf("\nParent: %s | Current State: %s", head->ParentIdentifier, GetStateNameString(head->CurrentState, stateStr));

        printf("\n");
        head = head->Next;
    }
}

void DisplayNFA(Picoc *pc) {
    struct Socket *head = pc->SocketList;
    char stateStr[20];

    while (head != NULL) {
        // printf("Socket FD %d - ", head->FileDescriptor);
        printf("Socket ID %s - ", head->Identifier);
        if (!strcmp(head->SocketType, "SOCK_STREAM")) {
            printf("TCP - ");
        } else if (!strcmp(head->SocketType, "SOCK_DGRAM")) {
            printf("UDP - ");
        } else if (!strcmp(head->SocketType, "")) {
            printf("Child of FD %d - ", head->ParentFileDescriptor);
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

        oldHead = oldSocketList;
        head = head->Next;
    }
}

enum SocketState MergeStates(enum SocketState state1, enum SocketState state2) {
    enum SocketState state;

    if (state1 == MayBeListening || state2 == MayBeListening) {
        state = MayBeListening;
    } else if (state1 == MayBeReadingOrWriting || state2 == MayBeReadingOrWriting) {
        state = MayBeReadingOrWriting;
    } else {
        state = state1;
    }

    return state;
}


void MergeSockets(Picoc *pc, struct Socket *oldSocketList) {
    struct Socket *head = pc->SocketList;
    struct Socket *oldHead = oldSocketList;
    // int newSocket;

    while(head != NULL) {
        // newSocket = 1;
        while(oldHead != NULL) {
            if (!strcmp(head->Identifier, oldHead->Identifier)) {
                head->CurrentState = MergeStates(head->CurrentState, oldHead->CurrentState);
                break;
            }

            oldHead = oldHead->Next;
        }

        oldHead = oldSocketList;
        head = head->Next;
    }
}

void UpdateCurrentState(Picoc *pc, char *identifier, const char *FuncName) {
    if (/*CheckIfWriteFunc(FuncName) || CheckIfReadFunc(FuncName) || */!strcmp("close", FuncName)) {
        return;
    }
    struct Socket *head = pc->SocketList; 
    struct Socket *socket = FindSocketByIdentifier(head, identifier);
    // struct Source *source = socket->SourceStack;
    // struct Source *newSource = (struct Source *) malloc(sizeof(struct Source));
    
    // newSource->State = FindState(FuncName);
    // newSource->Next = NULL;
    if (socket) {
        enum SocketState concreteState = FindState(FuncName);
        enum SocketState abstractState = StateAbstraction(concreteState);
        enum SocketState SocketCurrentState = socket->CurrentState;

        if (abstractState != -1)
            socket->CurrentState = MergeStates(SocketCurrentState, abstractState);
    }
    // socket->SourceStack = newSource;
    // while (source != NULL) {
    //     source->FindState(FuncName);
    // }
}

void AddId(Picoc *pc, char *identifier, int type) {
    struct Id *temp;
    int unique = 1;

    if (type == 0) {
        temp = pc->VarIdList;
    } else if (type == 1) {
        temp = pc->FuncIdList;
    }


    // if (temp == NULL) {
    //     unique = 1;
    // } else {
        while (temp != NULL) {
            if (!strcmp(temp->Identifier, identifier)) {
                unique = 0;
                break;
            }

            temp = temp->Next;
        }
    // }

    if (unique) {
        struct Id *newId = (struct Id *) malloc(sizeof(struct Id));

        newId->Identifier = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
        strcpy(newId->Identifier, identifier);


        if (type == 0) {
            newId->Next =  pc->VarIdList;
            pc->VarIdList = newId;
        } else if (type == 1) {
            newId->Next =  pc->FuncIdList;
            pc->FuncIdList = newId;
        }
    }
}

void DisplayIdList(struct Id *IdList) {
    struct Id *temp = IdList;

    while (temp != NULL) {
        printf("%s ", temp->Identifier);

        temp = temp->Next;
    }

    printf("\n");
}

void AddCharacteristic(Picoc *pc, int type, int line) {
    struct Characteristic *temp = pc->CharacteristicList;
    int distinct = 1;

    while (temp != NULL) {
        if (temp->CharacteristicType == type && temp->Line == line) {
            distinct = 0;
            break;
        }

        temp = temp->Next;
    }

    if (distinct) {
        struct Characteristic *newCharacteristic = (struct Characteristic *) malloc(sizeof(struct Characteristic));

        newCharacteristic->CharacteristicType = type;
        newCharacteristic->Line = line;
        newCharacteristic->Next = pc->CharacteristicList;

        pc->CharacteristicList = newCharacteristic;
    }
}

void UpdateDup(Picoc *pc, char *identifier, char *dup) {
    struct Socket *head = pc->SocketList; 
    struct Socket *socket = FindSocketByIdentifier(head, identifier);

    if (socket) {
        int dupFD = atoi(dup);
        socket->Dup2Arr[dupFD] += 1;
    }
}

char *GenerateDupListString(struct Socket *socket) {
    char *dupListStr = (char *)calloc(12, sizeof(char));
    char *str = (char *)calloc(5, sizeof(char));

    strcat(dupListStr, "[");

    for(int i = 0; i < 3; i++) {
        if (socket->Dup2Arr[i] > 0) {
            sprintf(str, "%d, ", i);
            strcat(dupListStr, str);
        }
    }

    if (strlen(dupListStr) > 1)
        dupListStr[strlen(dupListStr)-2] = '\0';
    
    strcat(dupListStr, "]");

    return dupListStr;
}

void GenerateForCmpReport(Picoc *pc) {
    struct Socket *socketHead = pc->SocketList;
    struct Characteristic *characteristicHead = pc->CharacteristicList;
    char stateStr[20];

    printf("===ALL SOCKETS===\n");
    while (socketHead != NULL) {
        printf("%s:%s:%s:%s:%d\n",
            socketHead->Identifier, socketHead->ParentIdentifier, 
            GetStateNameString(socketHead->CurrentState, stateStr), GenerateDupListString(socketHead),
            socketHead->Line);

        socketHead = socketHead->Next;
    }

    printf("===MAY BE LISTENING SOCKETS===\n");
    socketHead = pc->SocketList;
    while (socketHead != NULL) {
        if (socketHead->CurrentState == MayBeListening)
            printf("%s:%s:%s:%s:%d\n",
                socketHead->Identifier, socketHead->ParentIdentifier, 
                GetStateNameString(socketHead->CurrentState, stateStr), GenerateDupListString(socketHead),
                socketHead->Line);

        socketHead = socketHead->Next;
    }

    printf("===FORK ON LINE===\n");
    while(characteristicHead != NULL) {
        if (characteristicHead->CharacteristicType == Fork)
            printf("%d\n", characteristicHead->Line);

        characteristicHead = characteristicHead->Next;
    }

    printf("===EXEC ON LINE===\n");
    characteristicHead = pc->CharacteristicList;
    while(characteristicHead != NULL) {
        if (characteristicHead->CharacteristicType == Exec)
            printf("%d\n", characteristicHead->Line);

        characteristicHead = characteristicHead->Next;
    }

    printf("===END===");

    // cause need exec info at the end as well
    // need to remove the non-essential print statements


}

void generateSocketJson(cJSON *socket_array, struct Socket *socketHead, int type) {
    cJSON *socket = NULL;
    cJSON *dup = NULL;
    cJSON *socket_attribute = NULL;
    char stateStr[20];

    while (socketHead != NULL) {
        if (type || socketHead->CurrentState != MayBeListening) {
            socket = cJSON_CreateObject();
            cJSON_AddItemToArray(socket_array, socket);

            socket_attribute = cJSON_CreateString(socketHead->Identifier);
            cJSON_AddItemToObject(socket, "identifier", socket_attribute);

            socket_attribute = cJSON_CreateString(socketHead->ParentIdentifier);
            cJSON_AddItemToObject(socket, "parent", socket_attribute);

            socket_attribute = cJSON_CreateString(GetStateNameString(socketHead->CurrentState, stateStr));
            cJSON_AddItemToObject(socket, "state", socket_attribute);

            socket_attribute = cJSON_CreateArray();
            cJSON_AddItemToObject(socket, "dup", socket_attribute);

            for(int i = 0; i < 3; i++) {
                if (socketHead->Dup2Arr[i] > 0) {
                    dup = cJSON_CreateNumber(i);
                    cJSON_AddItemToArray(socket_attribute, dup);
                }
            }

            socket_attribute = cJSON_CreateNumber(socketHead->Line);
            cJSON_AddItemToObject(socket, "line", socket_attribute);
        }
        
        socketHead = socketHead->Next;
    }
}

void generateCharacteristicJson(cJSON *characteristic_array, struct Characteristic *characteristicHead, enum CharacteristicType type) {
    cJSON *line = NULL;

    while(characteristicHead != NULL) {
        if (characteristicHead->CharacteristicType == type) {
            line = cJSON_CreateNumber(characteristicHead->Line);
            cJSON_AddItemToArray(characteristic_array, line);
        }

        characteristicHead = characteristicHead->Next;
    }
}

char *create_monitor(Picoc *pc)
{
    char *string = NULL;
    struct Socket *socketHead = pc->SocketList;
    struct Characteristic *characteristicHead = pc->CharacteristicList;

    cJSON *monitor = cJSON_CreateObject();
    cJSON *all_sockets = cJSON_CreateArray();
    cJSON *maybelistening_sockets = cJSON_CreateArray();
    cJSON *fork_arr = cJSON_CreateArray();
    cJSON *exec_arr = cJSON_CreateArray();

    cJSON_AddItemToObject(monitor, "all_sockets", all_sockets);
    cJSON_AddItemToObject(monitor, "maybelistening_sockets", maybelistening_sockets);
    cJSON_AddItemToObject(monitor, "fork", fork_arr);
    cJSON_AddItemToObject(monitor, "exec", exec_arr);
    generateSocketJson(all_sockets, socketHead, 1); // all sockets
    generateSocketJson(maybelistening_sockets, socketHead, 0); // may be listening sockets
    generateCharacteristicJson(fork_arr, characteristicHead, Fork);
    generateCharacteristicJson(exec_arr, characteristicHead, Exec);


    string = cJSON_Print(monitor);
    if (string == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");
    }

end:
    cJSON_Delete(monitor);
    return string;
}