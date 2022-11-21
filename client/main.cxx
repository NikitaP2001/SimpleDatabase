#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>

#include <error.hpp>

#include "client.h"

int __cdecl main(int argc, char **argv) 
{
        WSADATA wsaData;
        std::string errorStr;
        std::string host = "127.0.0.1";
        std::string port = "1337";
        std::vector<Column> res;
        int iResult = 0;

        iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (iResult != 0) {
                ERR("WSAStartup failed");
                exit(1);
        }

        Client clt(host, port);
        if (clt.initilized()) {
                if (clt.execute("select id, name from file", "testdb", res, errorStr)) {
                        for (auto &col : res) {
                                std::cout << col.name << std::endl;
                                for (auto &val : col.values) {
                                        std::cout << val->GetValue() << std::endl;
                                }
                        }
                } else {
                        std::cout << "error: " << errorStr << std::endl;
                }
                
        } else {
                ERR("error not initilized");
        }

        WSACleanup();
        exit(0);
}