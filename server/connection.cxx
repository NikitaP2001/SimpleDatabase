#include "connection.h"

using namespace connection;


Connection::Connection(SOCKET socket) noexcept
        : m_socket(socket)
{

}

Connection::~Connection()
{
        if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR) {
                ERR("shutdown failed");
        }
        closesocket(m_socket);
}


bool Connection::recvCall() noexcept
{
        bool status = false;
        INFO("call proceeded");
        return status;
}