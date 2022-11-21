#include <thread>
#include <mutex>

#include "lexer.h"
#include "database.h"
#include "parser.h"

#include "connection.h"

using namespace connection;

using namespace lexer;

Connection::Connection(SOCKET socket) noexcept
        : m_socket(socket),
        m_loopThread(&Connection::receiveAndExecute, this)
{

}


Connection::~Connection()
{
        if (m_socket != INVALID_SOCKET) {
                if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR)
                        ERR("shutdown failed");
                closesocket(m_socket);
        }
}


bool Connection::operator()() const noexcept
{
        return m_continue;
}

nlohmann::json Connection::receiveJson()
{
        int iResult;
        bool msgEnd = false;
        std::string buffer;
        std::unique_ptr<char[]> recvBuf = std::make_unique<char[]>(m_kRecvBufSize);
        do {
                iResult = recv(m_socket, recvBuf.get(), m_kRecvBufSize, 0);
                if (iResult > 0) {
                        if (recvBuf[iResult - 1] == '\0')
                                msgEnd = true;
                        else
                                recvBuf[iResult] = '\0';
                        buffer += recvBuf.get();
                } else if (iResult == 0) {
                        iResult = 0;
                        break;
                } else if (iResult < 0)
                        throw std::runtime_error("recv failed");
        } while(!msgEnd);
        nlohmann::json json = nlohmann::json::parse(buffer);
        return json;
}

bool Connection::execute(std::string command, 
                        std::string database,
                        std::vector<Column> &cols, 
                        std::string &strErr)
{
        bool status = false;
        Lexer lex;
        std::vector<lexer::token> tokens;
        Storage st(database.c_str());
        parser::Parser prs(std::make_shared<Storage>(st));
        lexer::token tok(stmt::type::type_error, token::value(""));
        auto sit = command.begin();
        while (sit != command.end()) {
                sit = lex.getToken(sit, command.end(), tok);
                if (sit == command.end()) {
                        strErr = "syntax error";
                        break;
                }
                sit++;
                tokens.push_back(tok);
        }

        auto itTokBeg = tokens.begin();
        auto root = 
        prs.getTree(stmt::type::query, itTokBeg, tokens.end());
        if (root) {
                try {
                        auto response = root->execute("");
                        Column col;
                        while (response.getColumn(col)) {
                                cols.push_back(col);
                        }
                        status = true;
                } catch (std::exception &ex) {
                        strErr = ex.what();
                }
        } else {
                strErr = "invalid query syntax";
        }

        return status;
}

bool Connection::sendCols(std::vector<Column> &cols)
{
        int iResult = 0;
        bool result = true;
        nlohmann::json json;

        for (auto &col : cols) {
                nlohmann::json jCol;
                for (auto &val : col.values)
                        jCol.emplace_back(val->GetValue());
                json[col.name]  = jCol;
        }

        result = sendJson(json);

        return result;
}

bool Connection::sendJson(const nlohmann::json &json)
{
        int iResult = 0;
        bool result = true;

        std::string &&strJson = json.dump();
        iResult = send(m_socket, strJson.c_str(), strJson.size() + 1, 0);
        if (iResult == SOCKET_ERROR) {
                ERR("send failed");
                result = false;
        }
        INFO("Bytes Sent: " << iResult);

        return result;
}

bool Connection::receiveAndExecute()
{
        std::string errStr;
        std::vector<Column> cols;
        nlohmann::json errJson;
        bool status = true;
        try {
                auto json = receiveJson();
                if (execute(json["command"],
                                json["database"],
                                cols,
                                errStr)) {
                        if (!sendCols(cols))
                                ERR("failed to send columns");
                } else
                        status = false;
        } catch (std::exception &ex) {
                ERR("recv err: " << ex.what());
                errStr = ex.what();
                status = false;
        }

        if (!status) {
                errJson["_error_"] = errStr;
                sendJson(errJson);
        }


        m_continue = false;
        return status;
}