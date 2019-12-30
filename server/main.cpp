#include <iostream>
#include <ws2tcpip.h>
#include <string>
#include <fstream>
#include <regex>
#include <thread>

#define BUFFERSIZE 50000
#define SLEEPTIME 200

class Server {

private:
    struct my_struct {
        SOCKET* Connections;
        unsigned int ClientCount = 0;
        int ID = 0;
        std::string name;
        int cfg = 0;
        int db = 0;
        int server = 0;
        std::string srv_ip;
        std::string srv_port;
        unsigned int max_connections = 3; //default value
    };

    struct my_struct serverdata;

    [[noreturn]] void forkeepalive() {
        for (;;Sleep(300000)) {
            std::string buff = "ForKeepAlive\n";
            Sleep(200);
            for (unsigned int i = 0; i <= serverdata.ClientCount; i++) {
                if(serverdata.Connections[i]) {
                    send(serverdata.Connections[i],buff.c_str(),static_cast<int>(buff.length()),0);
                }
            }
        }
    }

    void sendMessage(){
        int id = serverdata.ID;
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

public:

    void options() {
        const std::string preg_ip = "((25[0-5]|2[0-4]\\d|1[0-9]\\d|[1-9]?\\d)\\.){3}(25[0-5]|2[0-4]\\d|1[0-9]\\d|[1-9]\\d|[1-9]{1}$)";
        const std::string preg_port = "(6553[0-5]{1}$|655[0-2]\\d|65[0-4][0-9]\\d|6[0-4][0-9][0-9]\\d|^[1-5][0-9][0-9][0-9]\\d|^[1-9][0-9][0-9]\\d|^[1-9][0-9]\\d|[1-9]\\d|[1-9]{1}$)";
        const std::string preg_max_conn = "\\d{1,5}";
        std::ifstream read_cfg;
        read_cfg.open("srv.cfg");
        if (!read_cfg) {
            std::cout << "Configuration file 'srv.cfg' does not exist.\nCreate new?(default:yes): ";
            std::string answer;
            std::getline(std::cin,answer);
            if (answer == "y" || answer == "Y" || answer == "Yes" || answer == "yes" || answer == "YES" || answer == ""){
                std::ofstream write_cfg("srv.cfg", std::ofstream::trunc);
                std::string ip;
                std::string port;
                std::string max_conn;
                std::cout << "Enter server ip: ";
                int chek_ip = 0;
                while (!chek_ip) {
                    std::getline(std::cin,ip);
                    if (std::regex_match(ip, std::regex(preg_ip))) {
                        write_cfg << ip << std::endl;
                        chek_ip++;
                    } else {
                        std::cout << "The entered value does not comply with the ip v4 standard. Please enter a valid one.\n";
                        std::cout << "Enter server ip: ";
                    }
                }

                std::cout << "Enter server port (1-65535): ";
                int chek_port = 0;
                while (!chek_port) {
                    std::getline(std::cin,port);
                    if (std::regex_match(port, std::regex(preg_port))) {
                        write_cfg << port << std::endl;
                        chek_port++;
                    } else {
                        std::cout << "The entered value does not comply with the port standard. Please enter a valid one.\n";
                        std::cout << "Enter server port (1-65535): ";
                    }
                }

                std::cout << "Enter enter the maximum number of connections (0-99999): ";
                int chek_max_conn = 0;
                while (!chek_max_conn) {
                    std::getline(std::cin,max_conn);
                    if (std::regex_match(max_conn, std::regex(preg_max_conn))) {
                        write_cfg << max_conn;
                        chek_max_conn++;
                    } else {
                        std::cout << "Invalid value entered. Please enter a valid one.\n";
                        std::cout << "Enter the maximum number of connections (0-99999): ";
                    }
                }

                write_cfg.close();
                serverdata.cfg = 1;
                system("cls");
                read_cfg.open("srv.cfg");
            } else {
                return;
            }
        }

        std::string ip, port, max_conn, cfg;
        int i = 0;
        while (std::getline(read_cfg,cfg)) {
            if (i == 0) {
                ip = cfg;
                i++;
            } else if (i == 1) {
                port = cfg;
                i++;
            } else if (i == 2) {
                max_conn = cfg;
            }
        }

        read_cfg.close();

        if (!std::regex_match(ip, std::regex(preg_ip)) || !std::regex_match(port, std::regex(preg_port)) || !std::regex_match(max_conn, std::regex(preg_max_conn))) {
            std::cout << "Configuration file 'srv.cfg' is damaged.\nCreate new?(default:yes): ";
            std::string answer;
            std::getline(std::cin,answer);
            if (answer == "y" || answer == "Y" || answer == "Yes" || answer == "yes" || answer == "YES" || answer == ""){
                std::ofstream write_cfg("srv.cfg", std::ofstream::trunc);
                std::string ip;
                std::string port;
                std::cout << "Enter server ip: ";
                int chek_ip = 0;
                while (!chek_ip) {
                    std::getline(std::cin,ip);
                    if (std::regex_match(ip, std::regex(preg_ip))) {
                        write_cfg << ip << std::endl;
                        chek_ip++;
                    } else {
                        std::cout << "The entered value does not comply with the ip v4 standard. Please enter a valid one.\n";
                        std::cout << "Enter server ip: ";
                    }
                }

                std::cout << "Enter server port (1-65535): ";
                int chek_port = 0;
                while (!chek_port) {
                    std::getline(std::cin,port);
                    if (std::regex_match(port, std::regex(preg_port))) {
                        write_cfg << port << std::endl;
                        chek_port++;
                    } else {
                        std::cout << "The entered value does not comply with the port standard. Please enter a valid one.\n";
                        std::cout << "Enter server port (1-65535): ";
                    }
                }

                std::cout << "Enter enter the maximum number of connections (0-99999): ";
                int chek_max_conn = 0;
                while (!chek_max_conn) {
                    std::getline(std::cin,max_conn);
                    if (std::regex_match(max_conn, std::regex(preg_max_conn))) {
                        write_cfg << max_conn;
                        chek_max_conn++;
                    } else {
                        std::cout << "Invalid value entered. Please enter a valid one.\n";
                        std::cout << "Enter the maximum number of connections (0-99999): ";
                    }
                }

                write_cfg.close();
                serverdata.cfg = 1;
                system("cls");
                read_cfg.open("srv.cfg");
            } else {
                return;
            }
        }

        i = 0;

        while (std::getline(read_cfg,cfg)) {
            if (i == 0) {
                ip = cfg;
                i++;
            } else if (i == 1) {
                port = cfg;
                i++;
            } else if (i == 2) {
                max_conn = cfg;
            }
        }

        read_cfg.close();
        std::cout << "Configuration file is ok.\n";
        serverdata.cfg = 1;
        std::cout << "Server ip: " << ip << std::endl;
        std::cout << "Server port: " << port << std::endl;
        std::cout << "Maximum number of connections: " << max_conn << std::endl;
        serverdata.srv_ip = ip;
        serverdata.srv_port = port;
        read_cfg.close();
        std::string answer = "1";

        while (answer == "1") {
            std::cout << "You want to re enter srv.cfg data?(default:no): ";
            std::getline(std::cin,answer);
            if (answer == "y" || answer == "Y" || answer == "Yes" || answer == "yes" || answer == "YES"){
                answer = "1";
                std::ofstream write_cfg("srv.cfg", std::ofstream::trunc);
                std::cout << "Enter server ip: ";
                int chek_ip = 0;

                while (!chek_ip) {
                    std::getline(std::cin,ip);
                    if (std::regex_match(ip, std::regex(preg_ip))) {
                        write_cfg << ip << std::endl;
                        chek_ip++;
                    } else {
                        std::cout << "The entered value does not comply with the ip v4 standard. Please enter a valid one.\n";
                        std::cout << "Enter server ip: ";
                    }
                }

                std::cout << "Enter server port (1-65535): ";
                int chek_port = 0;

                while (!chek_port) {
                    std::getline(std::cin,port);
                    if (std::regex_match(port, std::regex(preg_port))) {
                        write_cfg << port << std::endl;
                        chek_port++;
                    } else {
                        std::cout << "The entered value does not comply with the port standard. Please enter a valid one.\n";
                        std::cout << "Enter server port (1-65535): ";
                    }
                }

                std::cout << "Enter enter the maximum number of connections (0-99999): ";
                int chek_max_conn = 0;

                while (!chek_max_conn) {
                    std::getline(std::cin,max_conn);
                    if (std::regex_match(max_conn, std::regex(preg_max_conn))) {
                        write_cfg << max_conn;
                        chek_max_conn++;
                    } else {
                        std::cout << "Invalid value entered. Please enter a valid one.\n";
                        std::cout << "Enter the maximum number of connections (0-99999): ";
                    }
                }

                write_cfg.close();
                system("cls");
                std::cout << "Configuration file is ok.\n";
                std::cout << "Server ip: " << ip << std::endl;
                std::cout << "Server port: " << port << std::endl;
                std::cout << "Maximum number of connections: " << max_conn << std::endl;
                serverdata.srv_ip = ip;
                serverdata.srv_port = port;
            }
        }
    }

    void chekDB() {
        if (serverdata.cfg){
            std::cout << "Test chekDB method\n";
        }
    }

    void status() {
        system("cls");
        if (serverdata.cfg) {
            std::cout << "Configuration file is ok.\n";
        } else {
            std::cout << "Configuration file 'srv.cfg' does not exist. Server cannot start\n";
        }
        if (serverdata.db) {
            std::cout << "DB file is ok.\n";
        } else {
            std::cout << "The database is not found or damaged, no user can connect to the server.\n";
        }
        if (serverdata.server) {
            std::cout << "Server is running.\n";
        } else {
            std::cout << "Server is not running.\n";
        }
    }

    void startServer() {
        options();
        if (serverdata.cfg){
            SOCKET Connect;
            SOCKET Listen;
            WSAData data;
            WORD version = MAKEWORD(2,2);
            WSAStartup(version,&data);
            struct addrinfo hints;
            struct addrinfo * result;
            serverdata.Connections = static_cast<SOCKET*>(calloc((serverdata.max_connections+1),sizeof(SOCKET)));
            ZeroMemory(&hints,sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_flags = AI_PASSIVE;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            getaddrinfo(nullptr,serverdata.srv_port.c_str(), &hints,&result);
            Listen = socket(result->ai_family, result->ai_socktype,result->ai_protocol);
            if (Listen) {
                serverdata.server = 1;
                bind(Listen,result->ai_addr,static_cast<int>(result->ai_addrlen));
                listen(Listen,SOMAXCONN);
                freeaddrinfo(result);
                std::thread t1(&Server::forkeepalive, this);
                status();
                for (;;Sleep(75)) {
                    if((Connect = accept(Listen,nullptr,nullptr)) != INVALID_SOCKET && serverdata.ClientCount < serverdata.max_connections) {
                        for (serverdata.ID = 0 ; serverdata.Connections[serverdata.ID] != 0;) {
                            serverdata.ID++;
                        }
                        serverdata.Connections[serverdata.ID] = Connect;
                        std::thread t2(&Server::sendMessage, this);
                        t2.detach();
                    }
                }

            } else {
                //logs socket err
            }
        }
        status();
    }
};

int main()
{
    Server srv;
    srv.startServer();
    return 0;
}
