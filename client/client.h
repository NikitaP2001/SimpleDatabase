#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <json.hpp>

#include <database.h>


class Client {

public:
        Client(std::string_view hostName, std::string_view port) noexcept;
        ~Client();

        bool execute(IN std::string query, IN std::string database, 
        OUT std::vector<Column> &res, OUT std::string &strErr);

        [[nodiscard]] bool initilized() const noexcept;

private:

        bool connectToHost() noexcept;

        bool recvResult(std::vector<Column> &cols, std::string &strErr);

        bool sendQuery(std::string query, std::string database);

        nlohmann::json receiveJson();

        void disconnect();

private:
        addrinfo *m_hostAddr;

        SOCKET m_connectSocket = INVALID_SOCKET;

        bool m_initilized = true;

        constexpr static int m_kRecvBufSize = 1024;


};