#include <iostream>

#include "server.h"

using namespace lexer;

::grpc::Status AddressBookService::Execute(::grpc::ServerContext* context, 
const ::db::DbQuery* request, ::db::Table* response)
{
	std::vector<Column> cols;
	std::string errStr;
	nlohmann::json resJson;
	std::cout << "Server: recvd command \"" << request->command() << "\"." << std::endl;
	nlohmann::json json = nlohmann::json::parse(request->command());
	
	bool status = true;
	try {
			if (execute(json["command"],
							json["database"],
							cols,
							errStr)) {
					resJson = packCols(cols);
			} else
					status = false;
	} catch (std::exception &ex) {
			std::cerr << ex.what();
			errStr = ex.what();
			status = false;
	}

	if (!status) {
			std::cout << "error" << errStr;
			resJson["_error_"] = errStr;
	}

	response->set_table(resJson.dump());
	
	return grpc::Status::OK;
}


nlohmann::json AddressBookService::packCols(std::vector<Column> &cols)
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

        return json;
}


bool AddressBookService::execute(std::string command, 
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