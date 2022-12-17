#pragma once

#include <json.hpp>

#include <DbValue.h>

namespace connection {

class Connection {

public:
        explicit Connection(SOCKET socket) noexcept;

        [[nodiscard]] bool operator()() const noexcept;

        ~Connection();

private:
        void receiveAndExecute();

        bool execute(IN std::string command, 
                        IN std::string database,
                        OUT std::vector<Column> &cols, 
                        OUT std::string &strErr);

        bool sendCols(std::vector<Column> &cols);

        bool sendJson(const nlohmann::json &json);

        nlohmann::json receiveJson();
private:

        SOCKET m_socket;

        bool m_continue = true;

        HANDLE m_loopThread;

        constexpr static int m_kRecvBufSize = 1024;
};

} // connection 
