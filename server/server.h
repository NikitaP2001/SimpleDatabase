#pragma once
#include <list>
#include <thread>
#include <mutex>
#include <string>

#include <json.hpp>

#include "connection.h"
#include "main.h"

namespace server {

class Server {

public:
        explicit Server(std::string_view port) noexcept;
        ~Server();

        bool waitConnections() noexcept;

        [[nodiscard]] bool initialized() const noexcept;

private:
        void runConnections() noexcept;

        bool receiveAndExecute();

        nlohmann::json receiveJson();
private:

        bool m_initialized = true;

        std::list<std::unique_ptr<connection::Connection>> m_connections;

        SOCKET m_listenSocket = INVALID_SOCKET;
};

} // server

