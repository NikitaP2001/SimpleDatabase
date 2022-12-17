#include "server.h"

using namespace server;

Server::Server(std::string_view port) noexcept
{
        DWORD status = 1;
        addrinfo *result = nullptr;
        addrinfo hints {};

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        status = getaddrinfo(NULL, port.data(), &hints, &result);
        if (status != 0) {
                ERR("getaddrinfo failed");
                m_initialized = false;
        } 
        
        if (initialized()) {
                m_listenSocket = socket(result->ai_family, result->ai_socktype,
                result->ai_protocol);
                if (m_listenSocket == INVALID_SOCKET) {
                        ERR("socket creation failed");
                        freeaddrinfo(result);
                        m_initialized = false;
                }
        }

        if (initialized()) {
                status = bind(m_listenSocket, result->ai_addr, result->ai_addrlen);
                if (status == SOCKET_ERROR) {
                        ERR("bind failed");
                        freeaddrinfo(result);
                        m_initialized = false;
                }
        }
        
        if (initialized()) {
                status = listen(m_listenSocket, SOMAXCONN);
                if (status == SOCKET_ERROR) {
                        ERR("listen failed");
                        m_initialized = false;
                }
                freeaddrinfo(result);
        }

}


bool Server::waitConnections() noexcept
{
        bool status = true;
        SOCKET clientSocket {};

        while (status) {				
                clientSocket = accept(m_listenSocket, NULL, NULL);
                if (clientSocket == INVALID_SOCKET) {
                        ERR("accept failed");
                        wchar_t *s = NULL;
                        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER 
                        | FORMAT_MESSAGE_FROM_SYSTEM 
                        | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, WSAGetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPWSTR)&s, 0, NULL);
                        fprintf(stderr, "%S\n", s);
                        status = false;
                        LocalFree(s);
                } else {
						INFO("connection accepted");
                        m_connections.push_back( std::make_unique<
                        connection::Connection>(clientSocket));
                }

                m_connections.remove_if([](auto &c) { return !c; });
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return status;
}


Server::~Server()
{
        if (m_listenSocket != INVALID_SOCKET)
                closesocket(m_listenSocket);
}


bool Server::initialized() const noexcept
{
        return m_initialized;
}