#include <vector>

#include "lexer.h"
#include "database.h"
#include "parser.h"
#include "error.hpp"

using namespace lexer;

int main()
{
        Lexer lex;
        Storage st("testdb");
        parser::Parser prs(std::make_shared<Storage>(st));
        std::vector<lexer::token> tokens;

        lexer::token tok(stmt::type::type_error, token::value(""));

        std::string squery;
        while (std::getline(std::cin, squery)) {
                tokens.clear();

                lexer::token tok(stmt::type::type_error, token::value(""));
                auto sit = squery.begin();
                while (sit != squery.end()) {
                        sit = lex.getToken(sit, squery.end(), tok);
                        if (sit == squery.end()) {
                                std::cout << "syntax error" << std::endl;
                                break;
                        }
                        sit++;
                        tokens.push_back(tok);
                }
                if (tokens.begin() != tokens.end()) {
                        auto itTokBeg = tokens.begin();
                        auto root = 
                        prs.getTree(stmt::type::query, itTokBeg, tokens.end());
                        if (root && itTokBeg == tokens.end()) {
                                try {
                                        auto response = root->execute("");
                                        Column col;
                                        while (response.getColumn(col)) {
                                                std::cout << "column: " << col.name 
                                                << std::endl;
                                                for (auto val : col.values) {
                                                        std::cout << val->GetValue() << " ";
                                                }
                                                std::cout << std::endl;
                                        }
                                } catch (std::exception &ex) {
                                        std::cout << "[-] " << ex.what() << std::endl;
                                }
                                
                        } else {
                                std::cout << "parsing error" << std::endl;
                        }
               }
                
        }
}