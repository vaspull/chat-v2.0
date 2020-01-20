#include <stdio.h>
#include <ws2tcpip.h>

int main()
{
    unsigned int connection_count = 0;
    SOCKET* Connections;
    u_long iMode = 1;
    SOCKET Connect;
    Connections = (SOCKET*)calloc(64,sizeof(SOCKET));
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
    unsigned int listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    bind(listen_socket,addr->ai_addr,(int)addr->ai_addrlen);
    listen(listen_socket, SOMAXCONN);
    ioctlsocket(listen_socket, (long)FIONBIO, &iMode);
    for (;;Sleep(1)) {
        int bytes_read, bytes_write;
        if((Connect = accept(listen_socket,NULL,NULL)) != INVALID_SOCKET) {
            Connections[connection_count++] = Connect;
        }
        for(unsigned int i = 0; i <= connection_count; ++i) {
            char buffer[500];
            for(unsigned int a = 0; a <= sizeof(buffer); ++a) {
                buffer[a] = '\0';
            }
            if (Connections[i] != 4294967295 && Connections[i] > 0) {
                if ((bytes_read = recv(Connections[i], buffer, sizeof(buffer),0)) != SOCKET_ERROR ) {
                    for (unsigned int b = 0;b <= connection_count;++b) {
                        if (Connections[b]) {
                            if (Connections[b]) {
                                if ((bytes_write = send(Connections[b],buffer,(int)strlen(buffer),0)) == -1) {
                                    closesocket(Connections[b]);
                                    Connections[b] = '\0';
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}