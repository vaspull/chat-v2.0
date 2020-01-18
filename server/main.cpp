#include <iostream>
#include <ws2tcpip.h>
#include <string>
#include <fstream>
#include <regex>
#include <thread>
#include <sqlite3.h>

#define BUFFERSIZE 50000
#define SLEEPTIME 200
#define FOR_KEEP_ALIVE_SLEEP_TIME 300000
#define CFG_PATH "srv.cfg"

class Server {

private:
    struct my_struct {
        SOCKET* Connections;

        int cfg = 0;
        int db = 0;
        int server = 1;
        unsigned int ID = 0;
        unsigned int ClientCount = 0;
        unsigned int listen_socket;
        unsigned int max_connections = 32;  //default value
        std::string srv_ip = "127.0.0.1";   //default value
        std::string srv_port = "7770";      //default value
        std::string echo_mode = "true";    //default value
        std::string silent_mode = "false";  //default value
    };

    struct my_struct serverdata;

    [[noreturn]] void forkeepalive() {
        for (;;Sleep(FOR_KEEP_ALIVE_SLEEP_TIME)) {
            std::string buff = "ForKeepAlive\n";
            for (unsigned int i = 0; i <= serverdata.ClientCount; i++) {
                if(serverdata.Connections[i]) {
                    send(serverdata.Connections[i],buff.c_str(),static_cast<int>(buff.length()),0);
                }
            }
        }
    }

    void send_echo(){
        unsigned int id = serverdata.ID;
        serverdata.ClientCount++;
        int stop = 1;
        for (;stop == 1;Sleep(SLEEPTIME)) {
            int bytes_read;
            char buff[BUFFERSIZE] = {0};
            if ((bytes_read = recv(serverdata.Connections[id], buff, BUFFERSIZE, 0)) > 0) {
                buff[bytes_read] = '\0';
                for (unsigned int i = 0; i <= serverdata.ClientCount; i++) {
                    if (serverdata.Connections[i]) {
                        send(serverdata.Connections[i], buff, static_cast<int>(strlen(buff)), 0);
                    }
                }
            } else {
                closesocket(serverdata.Connections[id]);
                serverdata.Connections[id] = 0;
                stop = 0;
                serverdata.ClientCount--;
            }
        }
    }

    void sendMessage(){
        unsigned int id = serverdata.ID;
        serverdata.ClientCount++;
        int stop = 1;
        for (;stop == 1;Sleep(SLEEPTIME)) {
            int bytes_read;
            char buff[BUFFERSIZE] = {0};
            if ((bytes_read = recv(serverdata.Connections[id], buff, BUFFERSIZE, 0)) > 0) {
                buff[bytes_read] = '\0';
                for (unsigned int i = 0; i <= serverdata.ClientCount; i++) {
                    if (serverdata.Connections[i] && i != id) {
                        send(serverdata.Connections[i], buff, static_cast<int>(strlen(buff)), 0);
                    }
                }
            } else {
                closesocket(serverdata.Connections[id]);
                serverdata.Connections[id] = 0;
                stop = 0;
                serverdata.ClientCount--;
            }
        }
    }

    int chek_ip (std::string ip) {
        const std::string preg_ip = "((25[0-5]|2[0-4]\\d|1[0-9]\\d|[1-9]?\\d)\\.){3}(25[0-5]|2[0-4]\\d|1[0-9]\\d|[1-9]\\d|[1-9]{1}$)";
        if (std::regex_match(ip, std::regex(preg_ip))) {
            return 1;
        }
        return 0;
    }

    int chek_port (std::string port) {
        const std::string preg_port = "(6553[0-5]{1}$|655[0-2]\\d|65[0-4][0-9]\\d|6[0-4][0-9][0-9]\\d|^[1-5][0-9][0-9][0-9]\\d|^[1-9][0-9][0-9]\\d|^[1-9][0-9]\\d|[1-9]\\d|[1-9]{1}$)";
        if (std::regex_match(port, std::regex(preg_port))) {
            return 1;
        }
        return 0;
    }

    int chek_max_conn (std::string max_conn) {
        const std::string preg_max_conn = "\\d{1,5}";
        if (std::regex_match(max_conn, std::regex(preg_max_conn))) {
            return 1;
        }
        return 0;
    }

    int chek_silent_mode (std::string silent_mode) {
        if (silent_mode == "false" || silent_mode == "true") {
            return 1;
        }
        return 0;
    }

    int chek_echo_mode (std::string echo_mode) {
        if (echo_mode == "false" || echo_mode == "true") {
            return 1;
        }
        return 0;
    }

    void extract_opt(std::string &ip, std::string &port, std::string &max_conn, std::string &echo_mode, std::string &silent_mode) {
        std::ifstream read_cfg(CFG_PATH);
        std::string cfg;
        while (std::getline(read_cfg,cfg)) {
            while (cfg.find(' ', 0) != std::string::npos || cfg.find('\t') != std::string::npos) {
                if (cfg.find(' ', 0) != std::string::npos) {
                    cfg.erase(cfg.find(' '), 1);
                } else if (cfg.find('\t') != std::string::npos) {
                    cfg.erase(cfg.find('\t'), 1);
                }
            }

            std::string temp = cfg;

            if (cfg.find('=') != std::string::npos) {
                temp.erase(cfg.find('='));
                cfg.erase(0,cfg.find('=')+1);
                if (temp == "server_ip") {
                    ip = cfg;
                } else if (temp == "server_port") {
                    port = cfg;
                } else if (temp == "max_connections") {
                    max_conn = cfg;
                } else if (temp == "echo_mode") {
                    echo_mode = cfg;
                } else if (temp == "silent_mode") {
                    silent_mode = cfg;
                }
            }
        }
    }

    void write_to_cfg () {
        system("cls");
        std::ofstream write_cfg(CFG_PATH, std::ofstream::trunc);
        std::string ip;
        std::string port;
        std::string max_conn;
        std::string echo_mode;
        std::string silent_mode;
        std::cout << "Enter server ip: ";
        int chek = 0;
        while (!chek) {
            std::getline(std::cin,ip);
            if (chek_ip(ip)) {
                write_cfg << "server_ip = " << ip << std::endl;
                chek++;
            } else {
                std::cout << "The entered value does not comply with the ip v4 standard. Please enter a valid one.\n";
                std::cout << "Enter server ip: ";
            }
        }

        chek = 0;
        std::cout << "Enter server port (1-65535): ";
        while (!chek) {
            std::getline(std::cin,port);
            if (chek_port(port)) {
                write_cfg << "server_port = " << port << std::endl;
                chek++;
            } else {
                std::cout << "The entered value does not comply with the port standard. Please enter a valid one.\n";
                std::cout << "Enter server port (1-65535): ";
            }
        }

        chek = 0;
        std::cout << "Enter enter the maximum number of connections (0-99999): ";
        while (!chek) {
            std::getline(std::cin,max_conn);
            if (chek_max_conn(max_conn)) {
                write_cfg << "max_connections = " << max_conn  << std::endl;
                chek++;
            } else {
                std::cout << "Invalid value entered. Please enter a valid one.\n";
                std::cout << "Enter the maximum number of connections (0-99999): ";
            }
        }

        chek = 0;
        std::cout << "Want to use the server in echo mode?(default:no): ";
        while(!chek) {
            std::getline(std::cin,echo_mode);
            if (echo_mode == "y" || echo_mode == "Y" || echo_mode == "yes" || echo_mode == "YES" || echo_mode == "n" || echo_mode == "N" || echo_mode == "no" || echo_mode == "NO" || echo_mode == "") {
                if (echo_mode == "n" || echo_mode == "N" || echo_mode == "no" || echo_mode == "NO" || echo_mode == "") {
                    write_cfg << "echo_mode = false" << std::endl;
                    chek++;
                } else {
                    write_cfg << "echo_mode = true" << std::endl;
                    chek++;
                }
            } else {
                std::cout << "Invalid value entered. Please enter 'yes' or 'no'.\n";
                std::cout << "Want to use the server in echo mode?(default:no): ";
            }
        }

        chek = 0;
        std::cout << "Want to use silent startup mode?(default:no): ";
        while(!chek) {
            std::getline(std::cin,silent_mode);
            if (silent_mode == "y" || silent_mode == "Y" || silent_mode == "yes" || silent_mode == "YES" || silent_mode == "n" || silent_mode == "N" || silent_mode == "no" || silent_mode == "NO" || silent_mode == "") {
                if (silent_mode == "n" || silent_mode == "N" || silent_mode == "no" || silent_mode == "NO" || silent_mode == "") {
                    write_cfg << "silent_mode = false" << std::endl;
                    chek++;
                } else {
                    write_cfg << "silent_mode = true" << std::endl;
                    chek++;
                }
            } else {
                std::cout << "Invalid value entered. Please enter 'yes' or 'no'.\n";
                std::cout << "Want to use silent startup mode?(default:no): ";
            }
        }

        write_cfg.close();
        serverdata.cfg = 1;
    }

    void refresh_serverdata (std::string ip, std::string port, std::string max_conn, std::string echo_mode, std::string silent_mode) {
        serverdata.srv_ip = ip;
        serverdata.srv_port = port;
        serverdata.max_connections = static_cast<unsigned int>(atoi(max_conn.c_str()));
        serverdata.echo_mode = echo_mode;
        serverdata.silent_mode = silent_mode;
    }

    [[noreturn]]    void accept_incoming () {
        SOCKET Connect;
        for (;;Sleep(75)) {
            if((Connect = accept(serverdata.listen_socket,nullptr,nullptr)) != INVALID_SOCKET && serverdata.ClientCount < serverdata.max_connections) {
                for (serverdata.ID = 0 ; serverdata.Connections[serverdata.ID] != 0;) {
                    serverdata.ID++;
                }
                serverdata.Connections[serverdata.ID] = Connect;
                std::thread t2(&Server::sendMessage, this);
                t2.detach();
            }
        }
    }

    int db() {

        const char* _SQLquery= "CREATE TABLE IF NOT EXISTS test(a,b,c); INSERT INTO test VALUES(1,2,3);";
        sqlite3 *db = nullptr; // хэндл объекта соединение к БД
        char *err = nullptr;
        // open connection
        if (sqlite3_open("test_database.db", &db)) {
            fprintf(stderr, "Error create DB: %s\n", sqlite3_errmsg(db));
        } else if (sqlite3_exec(db, _SQLquery, nullptr, nullptr, &err)) {
            // execute _SQLquery
            fprintf(stderr, "Error SQL query: %sn", err);
            sqlite3_free(err);
        }
        // close connеction
        sqlite3_close(db);
        fprintf(stderr, "error SQL: %sn", err);
        return 0;
    }

public:

    void options() {                                                                                                        //chek or create configuration file
        system("cls");
        std::ifstream read_cfg(CFG_PATH);
        std::string ip, port, max_conn, cfg, echo_mode, silent_mode;
        if (!read_cfg.is_open()) {
            std::cout << "Configuration file 'srv.cfg' does not exist.\nCreate new?(default:yes): ";
            std::string answer;
            std::getline(std::cin,answer);
            if (answer == "y" || answer == "Y" || answer == "Yes" || answer == "yes" || answer == "YES" || answer == ""){
                read_cfg.close();
                write_to_cfg();
            } else {
                read_cfg.close();
                return;
            }
        }

        extract_opt(ip, port, max_conn, echo_mode, silent_mode);

        if (!chek_ip(ip) || !chek_port(port) || !chek_max_conn(max_conn) || !chek_silent_mode(silent_mode) || !chek_echo_mode(echo_mode)) {
            std::cout << "Configuration file 'srv.cfg' is damaged.\nCreate new?(default:yes): ";
            std::string answer;
            std::getline(std::cin,answer);
            if (answer == "y" || answer == "Y" || answer == "Yes" || answer == "yes" || answer == "YES" || answer == "") {
                write_to_cfg();
            } else {
                return;
            }
        } else {
            serverdata.cfg = 1;
            system("cls");
            if (silent_mode == "false") {
                std::cout << "Configuration file is ok.\n";
                std::cout << "Server ip: " << ip << std::endl;
                std::cout << "Server port: " << port << std::endl;
                std::cout << "Maximum number of connections: " << max_conn << std::endl;
                std::cout << "Echo mode: " << echo_mode << std::endl;
                std::cout << "Silent mode: " << silent_mode << std::endl;
                std::string answer = "1";
                while (answer == "1") {
                    std::cout << "You want to re enter srv.cfg data?(default:no): ";
                    std::getline(std::cin,answer);
                    if (answer == "y" || answer == "Y" || answer == "Yes" || answer == "yes" || answer == "YES"){
                        answer = "1";
                        write_to_cfg();
                    } else {
                        extract_opt(ip, port, max_conn, echo_mode, silent_mode);
                        refresh_serverdata(ip, port, max_conn, echo_mode, silent_mode);
                    }
                }
            } else if (silent_mode == "true"){
                extract_opt(ip, port, max_conn, echo_mode, silent_mode);
                refresh_serverdata(ip, port, max_conn, echo_mode, silent_mode);
            }
        }
    }

    void chekDB() {
        if (serverdata.cfg){
            std::cout << "Test chekDB method\n";
        }
    }

    [[noreturn]]   void status() {
        while(true) {
            system("cls");
            if (serverdata.cfg) {
                std::cout << "Configuration file is ok.\n";
            } else {
                std::cout << "Configuration file 'srv.cfg' does not exist. Used default values\n";
            }
            if (serverdata.db) {
                std::cout << "DB file is ok.\n";
            } else {
                std::cout << "The database is not found or damaged, no user can connect to the server.\n";
            }
            if (serverdata.server) {
                std::cout << "Server is running.\n";
                std::cout << "Users connected: " << serverdata.ClientCount << "/" << serverdata.max_connections << std::endl;
            } else {
                std::cout << "Server is not running. Check the error log file.\n";
            }
            Sleep(5000);
        }
    }

    void start() {

        //db();
        options();

        SOCKET Connect;
        serverdata.Connections = static_cast<SOCKET*>(calloc((serverdata.max_connections+1),sizeof(SOCKET)));
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << "\n";
            serverdata.server = 0;
        }

        struct addrinfo* addr = nullptr;
        struct addrinfo hints;
        ZeroMemory(&hints,sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;
        result = getaddrinfo(serverdata.srv_ip.c_str(),serverdata.srv_port.c_str(), &hints, &addr);

        if (result != 0) {
            std::cerr << "getaddrinfo failed: " << result << "\n";
            WSACleanup();
            serverdata.server = 0;
        }

        unsigned int listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (listen_socket == INVALID_SOCKET) {
            std::cerr << "Error at socket: " << WSAGetLastError() << "\n";
            freeaddrinfo(addr);
            WSACleanup();
            serverdata.server = 0;
        }

        result = bind(listen_socket,addr->ai_addr,static_cast<int>(addr->ai_addrlen));

        if (result == SOCKET_ERROR) {
            std::cerr << "bind failed with error: " << WSAGetLastError() << "\n";
            freeaddrinfo(addr);
            closesocket(listen_socket);
            WSACleanup();
            serverdata.server = 0;
        }

        if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "listen failed with error: " << WSAGetLastError() << "\n";
            closesocket(listen_socket);
            WSACleanup();
            serverdata.server = 0;
        }

        std::thread t1(&Server::status, this);
        t1.detach();

        if (serverdata.server == 1) {
            std::thread t2(&Server::forkeepalive, this);
            t2.detach();
            for (;;Sleep(75)) {
                if((Connect = accept(listen_socket,nullptr,nullptr)) != INVALID_SOCKET && serverdata.ClientCount < serverdata.max_connections) {
                    for (serverdata.ID = 0 ; serverdata.Connections[serverdata.ID] != 0;) {
                        serverdata.ID++;
                    }
                    serverdata.Connections[serverdata.ID] = Connect;
                    if(serverdata.echo_mode == "true") {
                        std::thread t3(&Server::send_echo, this);
                        t3.detach();
                    } else if (serverdata.echo_mode == "false") {
                        std::thread t3(&Server::sendMessage, this);
                        t3.detach();
                    }
                }
            }
        }
    }
};

int main()
{
    Server srv;
    srv.start();
    return 0;
}
