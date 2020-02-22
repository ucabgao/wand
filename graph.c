#include <sys/socket.h>
#include "interpreter.h"


/* socket states */
enum SocketState
{
    Creation,       /* socket() */
    Binding,        /* bind() (only applicable to TCP) */
    Listening,      /* listen() (only applicable to TCP) */
    Open,           /* accept() (only applicable to TCP) */
    Close,          /* close() */
};

/* structure of a socket state graph node*/
struct SocketStateGraph
{
    enum SocketState State;         /* state of the socket */
    short int Line;                  /* line number of the function call */
    struct SocketStateGraph *Next;  /* next (actually previous) node in the graph) */
};

/* structure of a socket node */
struct Socket
{
    int FileDescriptor;                     /* file descriptor of the socket */
    int Type;                               /* socket type (e.g. SOCK_STREAM or SOCK_DGRAM) */
    struct SocketStateGraph *StateGraph;    /* the state graph of the socket (or are we just interested in the current state?) */
    struct Socket *Next;                /* next socket in the list */
};

/*  */
void SocketInit(Picoc *pc)
{
    // struct Socket *newSocket = (struct Socket *) malloc(sizeof(struct Socket));
    // newSocket->FileDescriptor = -1;
    // newSocket->Next = NULL;

    pc->SocketList = NULL;
}

void AddSocket(Picoc *pc, int fd, int type, short int line) {
    struct Socket *head = pc->SocketList;
    struct Socket *newSocket = (struct Socket *) malloc(sizeof(struct Socket));
    struct SocketStateGraph *newSocketStateGraph = (struct SocketStateGraph *) malloc(sizeof(struct SocketStateGraph));

    newSocketStateGraph->State = Creation;
    newSocketStateGraph->Line = line;
    newSocketStateGraph->Next = NULL;

    newSocket->FileDescriptor = fd;
    newSocket->Type = type;
    newSocket->StateGraph = newSocketStateGraph;
    newSocket->Next = head;

    pc->SocketList = newSocket;
}

struct Socket *FindSocket(struct Socket *s, int fd) {
    while(s != NULL) {
        if (s->FileDescriptor == fd) {

            return s;
        }
        s = s->Next;
    }

    return NULL;
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