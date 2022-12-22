#include "client.h"

Client::Client(std::string database, std::string_view serverIp, std::string_view serverPort)
	: m_dbName(database)
{

}


nlohmann::json Client::Execute(std::string_view command)
{	
	auto pckdCommand = packCommand(command);
	
	db::DbQuery query;
    db::Table result;
    query.set_command(pckdCommand.dump());
	
	auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    std::unique_ptr<db::DbService::Stub> stub = db::DbService::NewStub(channel);
    grpc::ClientContext context;
    grpc::Status status = stub->Execute(&context, query, &result);
	
	std::string packedTable = result.table();
	nlohmann::json json = nlohmann::json::parse(result.table());
	return json;
}


nlohmann::json Client::packCommand(std::string_view command)
{
		nlohmann::json json;
        json["command"] = command;
        json["database"] = m_dbName;
		return json;
}