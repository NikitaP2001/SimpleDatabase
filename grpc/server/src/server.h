#pragma once

#include <myproto/address.pb.h>
#include <myproto/addressbook.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include "json.hpp"

#include "lexer.h"
#include "database.h"
#include "parser.h"

class AddressBookService final : public db::DbService::Service {
	
public:

	virtual ::grpc::Status Execute(::grpc::ServerContext* context, const ::db::DbQuery* request, 
	::db::Table* response);
	
private:

	bool execute(std::string command, 
                        std::string database,
                        std::vector<Column> &cols, 
                        std::string &strErr);
						
	nlohmann::json packCols(std::vector<Column> &cols);
};
