#include <stdio.h>
#include <ws2tcpip.h>
#define BUFFSIZE 500
#define MAXCONN 32
#define MAX_GETLINE 500
#define OPT 200
#define CFG "srv.cfg"

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

/*возвращает позицию искомого символа 'c' в массиве buf,
 * если искомого символа в массиве нет, то возвращает -1*/
int finde(char c, char *buf) {
    for (int i = 0; i < (int)strlen(buf); ++i) {
        if (buf[i] == c) {
            return i;
        }
    }
    return -1;
}

/* если count = -1, то удалется все что правее символа в позиции int pos,
 * если 0, то все что левее,
 * если другое значение, то удаляется то количество символом которое задано count начиная с символа в позиции pos*/

void clear(u_int size, char *buffer) {
    for (u_int i = 0; i <= size; i++ ) {
        buffer[i] = '\0';
    }
}

void erase(int count, int pos, char *buf) {
    char tmp2[OPT];
    clear(OPT, tmp2);
    int i = 0;
    if (pos >= 0 ) {
        if (count == -1) {
            for (; i < pos; ++i) {
                tmp2[i] = buf[i];
            }
            strncpy(buf,tmp2,OPT);
            buf[strlen(buf)] = '\0';
        } else if (count == 0) {
            for (u_int a = 0; pos < (int)strlen(buf);) {
                tmp2[a++] = buf[pos++];
            }
            strncpy(buf,tmp2,OPT);
            buf[strlen(buf)] = '\0';
        } else {
            for (;i < pos; ++i) {
                tmp2[i] = buf[i];
            }
            for (;i < ((int)strlen(buf)-count); ++i) {
                tmp2[i] = buf[i+count];
            }
            strncpy(buf,tmp2,OPT);
            buf[strlen(buf)] = '\0';
        }
    }
}


char *getline () {
    char *buff = (char*)malloc(MAX_GETLINE);
    for(u_int i = 0; i <= sizeof(buff); i++) {
        buff[i] ='\0';
    }
    int c = getchar();
    for(u_int i = 0; c != '\n'; ++i) {
        if ( i == MAX_GETLINE) {
            buff[i] = '\0';
            return buff;
        } else {
            buff[i] = (char)c;
            c = getchar();
            if (c == '\n'){
                buff[++i] = '\0';
            }
        }
    }
    return buff;
}

void extract_opt(char *ip, char *port, char *max_conn, char *echo_mode, char *silent_mode) {
    FILE *cfg_file;
    cfg_file = fopen(CFG, "r");
    char tmp[OPT], cfg[OPT];
    for(u_int i = 0; i < OPT; i++) {
        tmp[i] = '\0';
        cfg[i] = '\0';
    }
    while (fgets(cfg,OPT,cfg_file) != NULL) {
        while (finde(' ', cfg) != -1) {
            erase(1,finde(' ', cfg),cfg);
        }
        while (finde('\t', cfg) != -1) {
            erase(1,finde('\t', cfg),cfg);
        }
        while (finde('\n', cfg) != -1) {
            erase(1,finde('\n', cfg),cfg);
        }
        strcpy(tmp,cfg);
        if (finde('=', cfg) != -1) {
            erase(-1,finde('=',cfg),tmp);
            erase(0,finde('=',cfg)+1, cfg);
            if (strncmp(tmp, "server_ip", 9) == 0) {
                strcpy(ip,cfg);
            } else if (strncmp(tmp, "server_port", 11) == 0) {
                strcpy(port,cfg);
            } else if (strncmp(tmp, "max_connections", 15) == 0) {
                strcpy(max_conn,cfg);
            } else if (strncmp(tmp, "echo_mode", 9) == 0) {
                strcpy(echo_mode,cfg);
            } else if (strncmp(tmp, "silent_mode", 11) == 0) {
                strcpy(silent_mode,cfg);
            }
        }
    }
    fclose(cfg_file);
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
        Sleep(10);
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
        Sleep(10);
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
