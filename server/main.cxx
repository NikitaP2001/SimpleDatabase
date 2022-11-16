#include <iostream>

#include "main.h"
#include "server.h"

int main()
{
        WSADATA data {};
        int res = 0;

        res = WSAStartup(MAKEWORD(2, 2), &data);
        if (res) {
                ERR("ws startup failed with error: " << res);
                exit(1);
        }

        std::string portNumber = "1337";
        auto srv = std::make_unique<server::Server>(portNumber);
        if (srv->initialized()) {
                srv->waitConnections();
        } else {
                ERR("server initialization failture");
        }

        WSACleanup();
        return res;
}