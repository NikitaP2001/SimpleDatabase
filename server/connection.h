#pragma once
#include "main.h"

namespace connection {

class Connection {

public:
        explicit Connection (SOCKET socket) noexcept;

        [[nodiscard]] bool recvCall() noexcept;

        ~Connection();
private:
        SOCKET m_socket;
};

} // connection 