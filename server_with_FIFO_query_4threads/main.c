#include <stdio.h>
#include <ws2tcpip.h>
#define BUFFSIZE 500
#define MAXCONN 32

typedef struct _Elem{
    char data[BUFFSIZE];
    struct _Elem* next;
    struct _Elem* prev;
}Elem;

typedef struct _List{
    u_int counter;
    struct _Elem* first;
    struct _Elem* last;
}List;

typedef struct _Server{
    SOCKET *Connections;
    List *query;
    u_int connection_count;
}Server;

List* createList() {
    List *query = (List*) malloc(sizeof(List));
    query->counter = 0;
    query->first = NULL;
    query->last = NULL;
    return query;
}

void add_to_end(List *query, char *data) {
    Elem *newelem = (Elem*) malloc(sizeof(Elem));
    for (u_int i = 0; i <= BUFFSIZE; i++) {
        newelem->data[i] = '\0';
    }
    strcpy(newelem->data, data);
    newelem->next = NULL;
    query->counter++;
    if (query->first == NULL) {
        query->first = newelem;
        newelem->prev = NULL;
        query->last = query->first;
    } else {
        newelem->prev = query->last;
        query->last->next = newelem;
        query->last = newelem;
    }
}

void delete_first(List *query) {
    if (!query->counter){
    } else if(query->first == query->last) {
        Elem *delelem = query->first;
        query->first-> next = NULL;
        query->first-> prev = NULL;
        query->first = NULL;
        query->last = NULL;
        free(delelem);
        free(query->first);
        --query->counter;
    } else if (query->first->next) {
        Elem *delelem = query->first;
        query->first = query->first->next;
        query->first->prev = NULL;
        free(delelem);
        free(query->first);
        --query->counter;
    }
}

char *return_first(List *query) {
    Elem *info = query->first;
    if (info) {
        char *tmp = info->data;
        return tmp;
    }
    free(info);
    return 0;
}

__attribute__ ((noreturn)) void recieve_messages(Server *server) {
    u_int count = 0;
    int bytes_read;
    char buf[BUFFSIZE];
    while(TRUE) {
        if (count < MAXCONN) {
            if (server->Connections[count]) {
                for(u_int i = 0; i <= BUFFSIZE; i++) {
                    buf[i] = '\0';
                }
                if((bytes_read = recv(server->Connections[count], buf, sizeof(buf),0)) != SOCKET_ERROR && bytes_read > 0 && bytes_read < BUFFSIZE){
                    add_to_end(server->query,buf);
                }
            }
            ++count;
        } else {
            count = 0;
        }
        Sleep(1);
    }
}

__attribute__ ((noreturn)) void send_messages(Server *server) {
    u_int count = 0;
    int bytes_write = 0;
    while(TRUE) {
        if (return_first(server->query)){
            if (count < MAXCONN) {
                if (server->Connections[count]){
                    if ((bytes_write = send(server->Connections[count], return_first(server->query), (int)strlen(return_first(server->query)),0)) == -1) {
                        closesocket(server->Connections[count]);
                        server->Connections[count] = 0;
                        --server->connection_count;
                    }
                }
                ++count;
            } else {
                count = 0;
                delete_first(server->query);
            }
        }
        Sleep(1);
    }
}

__attribute__ ((noreturn)) void clear_dead_conn(Server *server) {
    u_int count = 0;
    int bytes_write = 0;
    while(TRUE) {
        if (count < MAXCONN) {
            if (server->Connections[count]){
                if ((bytes_write = send(server->Connections[count], "", 1,0)) == -1) {
                    closesocket(server->Connections[count]);
                    server->Connections[count] = 0;
                    --server->connection_count;
                }
            }
            ++count;
        } else {
            count = 0;
        }
        Sleep(1000);
    }
}

int main()
{
    Server server;
    server.query = createList();
    u_long iMode = 1;
    SOCKET Connect;
    server.Connections = (SOCKET*)calloc(MAXCONN,sizeof(SOCKET));
    server.connection_count = 0;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    struct addrinfo* addr = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints,sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(0,"7770", &hints, &addr);
    u_int listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    bind(listen_socket,addr->ai_addr,(int)addr->ai_addrlen);
    listen(listen_socket, SOMAXCONN);
    ioctlsocket(listen_socket, (long)FIONBIO, &iMode);
    CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)*recieve_messages,(LPVOID)&server,0,0);
    CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)*send_messages,(LPVOID)&server,0,0);
    CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)*clear_dead_conn,(LPVOID)&server,0,0);
    for (;;Sleep(1)) {
        if (server.connection_count < MAXCONN) {
            if((Connect = accept(listen_socket,NULL,NULL)) != INVALID_SOCKET) {
                u_int x;
                for (x = 0; server.Connections[x]; ++x);
                server.Connections[x] = Connect;
                ++server.connection_count;
            }
        }
    }
}
