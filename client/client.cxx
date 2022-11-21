#include <memory>
#include <stdexcept>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <error.hpp>

#include "client.h"

Client::Client(std::string_view hostName, std::string_view port) noexcept
{
        int iResult = 0;
        addrinfo hints {};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        iResult = getaddrinfo(hostName.data(), port.data(), 
        &hints, &m_hostAddr);
        if (iResult != 0) {
                ERR("getaddrinfo failed");
                m_initilized = false;
        }
}

bool Client::sendQuery(std::string query, std::string database)
{
        int iResult = 0;
        bool result = true;
        nlohmann::json json;

        json["command"] = query;
        json["database"] = database;

        std::string &&strJson = json.dump();
        iResult = send(m_connectSocket, strJson.c_str(), strJson.size(), 0);
        if (iResult == SOCKET_ERROR) {
                ERR("send failed");
                result = false;
        }
        INFO("Bytes Sent: " << iResult);

        return result;
}

nlohmann::json Client::receiveJson()
{
        int iResult;
        bool msgEnd = false;
        std::string buffer;
        std::unique_ptr<char[]> recvBuf = std::make_unique<char[]>(m_kRecvBufSize);
        do {
                iResult = recv(m_connectSocket, recvBuf.get(), m_kRecvBufSize, 0);
                if (iResult > 0) {
                        if (recvBuf[iResult - 1] == '\0')
                                msgEnd = true;
                        else
                                recvBuf[iResult] = '\0';
                        buffer += recvBuf.get();
                } else if (iResult == 0) {
                        iResult = 0;
                } else if (iResult < 0)
                        throw std::runtime_error("recv failed");
        } while(msgEnd);
        nlohmann::json json = nlohmann::json::parse(buffer);
        return json;
}

bool Client::recvResult(std::vector<Column> &cols, std::string &strErr)
{
        bool status = true;
        Column col;
        try {
                auto json = receiveJson();
                for (auto &[key, vals] : json.items()) {
                        if (key == "_error_") {
                                strErr = vals;
                                status = false;
                                break;
                        } else {
                                col.values.clear();
                                col.name = key;
                                for (auto &val : vals.items())
                                        col.values.push_back(getDbValue(val.value()));
                                cols.push_back(col);
                        }
                        
                }

        } catch (std::exception &ex) {
                ERR(ex.what());
                status = false;
        }
        return status;
}

bool Client::execute(std::string query, std::string database, 
std::vector<Column> &res, std::string &strErr)
{
        bool status = true;

        if (!connectToHost())
                throw std::runtime_error("failed to connect");

        if (sendQuery(query, database)) {
                if (!recvResult(res, strErr)) {
                        ERR("error receiving result");
                        status = false;
                }

        } else {
                ERR("error sending query");
                status = false;
        }
        
        disconnect();

        return status;
}


bool Client::initilized() const noexcept
{
        return m_initilized;
}


bool Client::connectToHost() noexcept
{
        bool status = false;
        int iResult = 0;

        for(addrinfo *ptr = m_hostAddr; ptr != nullptr; ptr = ptr->ai_next) {
                m_connectSocket  = socket(ptr->ai_family, ptr->ai_socktype, 
                ptr->ai_protocol);
                if (m_connectSocket == INVALID_SOCKET) {
                        ERR("socket failed");
                        break;
                }

                iResult = connect(m_connectSocket, ptr->ai_addr, ptr->ai_addrlen);
                if (iResult != SOCKET_ERROR) {
                        status = true;
                        break;
                }
                closesocket(m_connectSocket);
                m_connectSocket = INVALID_SOCKET;
        }
        return status;
}

void Client::disconnect()
{
        if (m_connectSocket != INVALID_SOCKET) {
                shutdown(m_connectSocket, SD_SEND);
                closesocket(m_connectSocket);
                m_connectSocket = INVALID_SOCKET;
        }
}

Client::~Client()
{
        disconnect(); 
                
        if (initilized()) {
                freeaddrinfo(m_hostAddr);
        }
}