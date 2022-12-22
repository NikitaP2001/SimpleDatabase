#pragma once
#include <string>

#include <myproto/address.pb.h>
#include <myproto/addressbook.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include "json.hpp"

#include "database.h"

class Client {
	
public:
	Client(std::string database, std::string_view serverIp, std::string_view serverPort);
	
	nlohmann::json Execute(std::string_view command);
	
private:

	nlohmann::json packCommand(std::string_view command);
	
private:

	std::string m_dbName;


};