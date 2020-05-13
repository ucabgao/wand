#include <sys/socket.h>
#include "interpreter.h"
#include "cjson/cJSON.h"

const char * INTERESTEDFUNC[] = { "socket", "bind", "listen", "accept", "close", "read", "recv", "recvfrom", "write", "send", "sendto", "dup2", "fork", "execl", "execlp", "execle", "execv", "execvp", "execvpe"};

// initialise absint related
void AbsIntInit(Picoc *pc)
{
    pc->SocketList = NULL;
    pc->IdentifierAssignedTo = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
    pc->VarIdList = NULL;
    pc->FuncIdList = NULL;
    pc->CharacteristicList = NULL;
}

// add socket to socket list
void AddSocket(Picoc *pc, char *identifier, char *sockettype, short int line, char* parent) {
    struct Socket *head = pc->SocketList;
    struct Socket *newSocket = (struct Socket *) malloc(sizeof(struct Socket));

    newSocket->CurrentState = Initial;

    if (strcmp(parent,"")) { // parent is not empty string
        newSocket->CurrentState = NotReadingOrWriting;
    }

    // assign socket details
    newSocket->Identifier = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
    strcpy(newSocket->Identifier, identifier);
    newSocket->SocketType = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
    strcpy(newSocket->SocketType, sockettype);
    newSocket->ParentIdentifier = (char *) calloc(MAX_IDENTIFIER_SIZE, sizeof(char));
    strcpy(newSocket->ParentIdentifier, parent);
    newSocket->Line = line;
    newSocket->Dup2Arr = (int *) calloc(3, sizeof(int));

    newSocket->Next = head;

    pc->SocketList = newSocket;
}

// find socket by its identifier
struct Socket *FindSocketByIdentifier(struct Socket *s, char *identifier) {
    while(s != NULL) {
        if (!strcmp(s->Identifier, identifier)) {

            return s;
        }
        s = s->Next;
    }

    return NULL;
}

// find concrete state based on function
enum SocketState FindState(const char *FuncName) {
    enum SocketState s = -1;

    if (!strcmp(FuncName, "bind")) {
        s = Binding;
    } else if (!strcmp(FuncName, "listen") || !strcmp(FuncName, "accept")) {
        s = Listening;
    } else if (!strcmp(FuncName, "close")) {
        s = Closed;
    } else if (CheckIfReadFunc(FuncName)) {
        s = Reading;
    } else if (CheckIfWriteFunc(FuncName)) {
        s = Writing;
    }

    return s;
}

// get string of state
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

// display socket information
void DisplaySocket(Picoc *pc) {
    struct Socket *head = pc->SocketList;

    while (head != NULL) {
        printf("\nSocket ID %s - ", head->Identifier);

        char stateStr[20];
        printf("\nParent: %s | Current State: %s", head->ParentIdentifier, GetStateNameString(head->CurrentState, stateStr));

        printf("\n");
        head = head->Next;
    }
}

// check if function is of interest
int CheckFuncOfInterest(const char *FuncName) {
    int len = sizeof(INTERESTEDFUNC)/sizeof(INTERESTEDFUNC[0]);

    for(int i = 0; i < len; ++i)
    {
        if(!strcmp(INTERESTEDFUNC[i], FuncName))
        {
            return TRUE;
        }
    }

    return FALSE;
}

// check if the functions are for reading
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

// check if the functions are for writing
int CheckIfWriteFunc(const char *FuncName) {
    char * fn [] = { "write", "send", "sendto" };
    int len = sizeof(fn)/sizeof(fn[0]);

    for(int i = 0; i < len; ++i)
    {
        if(!strcmp(fn[i], FuncName))
        {
            return TRUE;
        }
    }

    return FALSE;
}

// check if the functions are for executing
int CheckIfExecFunc(const char *FuncName) {
    char * fn [] = { "execl", "execlp", "execle", "execv", "execvp", "execvpe" };
    int len = sizeof(fn)/sizeof(fn[0]);

    for(int i = 0; i < len; ++i)
    {
        if(!strcmp(fn[i], FuncName))
        {
            return TRUE;
        }
    }

    return FALSE;
}

// abstraction function - maps concrete states to abstract states
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

// merge abstract state
enum SocketState MergeStates(enum SocketState state1, enum SocketState state2) {
    enum SocketState state;

    // if either one is MayBeListening, the evaluated state is MayBeListening
    if (state1 == MayBeListening || state2 == MayBeListening) { 
        state = MayBeListening;

    // if either one is MayBeReadingOrWriting, the evaluated state is MayBeReadingOrWriting
    } else if (state1 == MayBeReadingOrWriting || state2 == MayBeReadingOrWriting) {
        state = MayBeReadingOrWriting;

    } else {
        state = state1;
    }

    return state;
}

// update the abstract state
void UpdateCurrentState(Picoc *pc, char *identifier, const char *FuncName) {
    if (!strcmp("close", FuncName)) {
        return;
    }

    struct Socket *head = pc->SocketList; 
    struct Socket *socket = FindSocketByIdentifier(head, identifier);

    if (socket) {
        enum SocketState concreteState = FindState(FuncName);
        enum SocketState abstractState = StateAbstraction(concreteState);
        enum SocketState SocketCurrentState = socket->CurrentState;

        if (abstractState != -1)
            socket->CurrentState = MergeStates(SocketCurrentState, abstractState);
    }
}

// add either variable or function identifier to the respective lists
void AddId(Picoc *pc, char *identifier, int type) {
    struct Id *temp;
    int unique = 1;

    if (type == 0) {
        temp = pc->VarIdList;
    } else if (type == 1) {
        temp = pc->FuncIdList;
    }

    // if the identifier is not found in the list, add to the list
    while (temp != NULL) {
        if (!strcmp(temp->Identifier, identifier)) {
            unique = 0;
            break;
        }

        temp = temp->Next;
    }

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

// display the identifiers in the identifier list
void DisplayIdList(struct Id *IdList) {
    struct Id *temp = IdList;

    while (temp != NULL) {
        printf("%s ", temp->Identifier);

        temp = temp->Next;
    }

    printf("\n");
}

// add the respective characteristics to the characteristic list
void AddCharacteristic(Picoc *pc, int type, int line) {
    struct Characteristic *temp = pc->CharacteristicList;
    int distinct = 1;

    // if the characteristic-line pair is not found in the list, add to the list
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

// update the dup array of the socket
void UpdateDup(Picoc *pc, char *identifier, char *dup) {
    struct Socket *head = pc->SocketList; 
    struct Socket *socket = FindSocketByIdentifier(head, identifier);

    if (socket) {
        int dupFD = atoi(dup);
        socket->Dup2Arr[dupFD] += 1;
    }
}

// convert the dup array to a list string for JSON
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

// convert socket information to json
void GenerateSocketJson(cJSON *socket_array, struct Socket *socketHead, int type) {
    cJSON *socket = NULL;
    cJSON *dup = NULL;
    cJSON *socket_attribute = NULL;
    char stateStr[20];

    while (socketHead != NULL) {
        if (type || socketHead->CurrentState == MayBeListening) {
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

// convert characteristic information to json
void GenerateCharacteristicJson(cJSON *characteristic_array, struct Characteristic *characteristicHead, enum CharacteristicType type) {
    cJSON *line = NULL;

    while(characteristicHead != NULL) {
        if (characteristicHead->CharacteristicType == type) {
            line = cJSON_CreateNumber(characteristicHead->Line);
            cJSON_AddItemToArray(characteristic_array, line);
        }

        characteristicHead = characteristicHead->Next;
    }
}

// convert identifier list information to json
void GenerateIdJson(cJSON *id_array, struct Id *idHead) {
    cJSON *identifier = NULL;

    while (idHead != NULL) {
        identifier = cJSON_CreateString(idHead->Identifier);
        cJSON_AddItemToArray(id_array, identifier);

        idHead = idHead->Next;
    }
}

// generate the output in json format
char *GenerateOutputJson(Picoc *pc)
{
    char *string = NULL;
    struct Socket *socketHead = pc->SocketList;
    struct Characteristic *characteristicHead = pc->CharacteristicList;
    struct Id *varidHead = pc->VarIdList;
    struct Id *funcidHead = pc->FuncIdList;

    double time_spent = (pc->EndTime.tv_sec - pc->StartTime.tv_sec) +
                        (pc->EndTime.tv_nsec - pc->StartTime.tv_nsec) / 1000000000.0;

    // create json attributes and objects
    cJSON *monitor = cJSON_CreateObject();
    cJSON *all_sockets = cJSON_CreateArray();
    cJSON *maybelistening_sockets = cJSON_CreateArray();
    cJSON *fork_arr = cJSON_CreateArray();
    cJSON *exec_arr = cJSON_CreateArray();
    cJSON *varid_arr = cJSON_CreateArray();
    cJSON *funcid_arr = cJSON_CreateArray();
    cJSON *analysis_time = cJSON_CreateNumber(time_spent);

    cJSON_AddItemToObject(monitor, "all_sockets", all_sockets);
    cJSON_AddItemToObject(monitor, "maybelistening_sockets", maybelistening_sockets);
    cJSON_AddItemToObject(monitor, "fork", fork_arr);
    cJSON_AddItemToObject(monitor, "exec", exec_arr);
    cJSON_AddItemToObject(monitor, "var_id", varid_arr);
    cJSON_AddItemToObject(monitor, "func_id", funcid_arr);
    cJSON_AddItemToObject(monitor, "analysis_time", analysis_time);

    GenerateSocketJson(all_sockets, socketHead, 1); // all sockets
    GenerateSocketJson(maybelistening_sockets, socketHead, 0); // may be listening sockets
    GenerateCharacteristicJson(fork_arr, characteristicHead, Fork);
    GenerateCharacteristicJson(exec_arr, characteristicHead, Exec);
    GenerateIdJson(varid_arr, varidHead);
    GenerateIdJson(funcid_arr, funcidHead);

    string = cJSON_Print(monitor);
    if (string == NULL)
    {
        fprintf(stderr, "Failed to print monitor.\n");
    }

    cJSON_Delete(monitor);
    return string;
}