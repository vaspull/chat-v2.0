#include <iostream>
#include <ws2tcpip.h>
#include <string>
#include <fstream>
#include <regex>
#include <thread>

#define BUFFERSIZE 50000
#define SLEEPTIME 200
#define FOR_KEEP_ALIVE_SLEEP_TIME 300000
#define CFG_PATH "srv.cfg"

class Server {

private:
    struct my_struct {
        SOCKET* Connections;
        int ID = 0;
        int cfg = 0;
        int db = 0;
        int server = 0;
        unsigned int ClientCount = 0;
        unsigned int max_connections = 32;  //default value
        std::string srv_ip = "127.0.0.1";   //default value
        std::string srv_port = "7770";      //default value
        std::string echo_mode = "false";    //default value
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

    void extract_opt(std::string &ip, std::string &port, std::string &max_conn) {
        std::ifstream read_cfg(CFG_PATH);
        std::string cfg;
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
    }

    void extract_opt_2(std::string &ip, std::string &port, std::string &max_conn, std::string &echo_mode, std::string &silent_mode) {
        std::ifstream read_cfg(CFG_PATH);
        std::string cfg;
        while (std::getline(read_cfg,cfg)) {
            std::string temp = cfg;
            unsigned int pos = cfg.find(':');
            temp.erase(pos);
            cfg.erase(0,pos+1);
            std::cout << "temp2: " << temp << std::endl << cfg << std::endl;
            system("pause");
            if (temp == "server ip") {
                ip = cfg;
            } else if (temp == "port") {
                port = cfg;
            } else if (temp == "max connections") {
                max_conn = cfg;
            } else if (temp == "echo mode") {
                echo_mode = cfg;
            } else if (temp == "silent mode") {
                silent_mode = cfg;
            }
        }
    }

    void write_to_cfg () {
        system("cls");
        std::ofstream write_cfg(CFG_PATH, std::ofstream::trunc);
        std::string ip;
        std::string port;
        std::string max_conn;
        std::cout << "Enter server ip: ";
        int chek = 0;
        while (!chek) {
            std::getline(std::cin,ip);
            if (chek_ip(ip)) {
                write_cfg << ip << std::endl;
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
                write_cfg << port << std::endl;
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
                write_cfg << max_conn;
                chek++;
            } else {
                std::cout << "Invalid value entered. Please enter a valid one.\n";
                std::cout << "Enter the maximum number of connections (0-99999): ";
            }
        }
        write_cfg.close();
        serverdata.cfg = 1;
    }

public:

    void options() {                                                                                                        //chek or create configuration file
        std::ifstream read_cfg(CFG_PATH);
        //std::string ip, port, max_conn, cfg;
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

        extract_opt(ip, port, max_conn);
        //extract_opt_2(ip, port, max_conn, echo_mode, silent_mode); //for future

        if (!chek_ip(ip) || !chek_port(port) || !chek_max_conn(max_conn)) {
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
            std::cout << "Configuration file is ok.\n";
            std::cout << "Server ip: " << ip << std::endl;
            std::cout << "Server port: " << port << std::endl;
            std::cout << "Maximum number of connections: " << max_conn << std::endl;

            std::string answer = "1";

            while (answer == "1") {
                std::cout << "You want to re enter srv.cfg data?(default:no): ";
                std::getline(std::cin,answer);
                if (answer == "y" || answer == "Y" || answer == "Yes" || answer == "yes" || answer == "YES"){
                    answer = "1";
                    write_to_cfg();
                } else {
                    extract_opt(ip, port, max_conn);
                    serverdata.srv_ip = ip;
                    serverdata.srv_port = port;
                    serverdata.max_connections = static_cast<unsigned int>(atoi(max_conn.c_str()));
                }
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
            std::cout << "Configuration file 'srv.cfg' does not exist. Used default values\n";
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
            //logs err
        }
    }
};

int main()
{
    Server srv;
    srv.startServer();
    return 0;
}
