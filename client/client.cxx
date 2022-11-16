#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "client.h"

Client::Client()
{
        SOCKET ConnectSocket = INVALID_SOCKET;

        struct addrinfo *result = NULL,
                        *ptr = NULL,
                        hints;
        const char *sendbuf = "this is a test";
        char recvbuf[DEFAULT_BUFLEN];
        int iResult;
        int recvbuflen = DEFAULT_BUFLEN;
}

Response Client::execute();
