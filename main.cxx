#include <vector>

#include "lexer.h"
#include "database.h"
#include "parser.h"
#include "error.hpp"

using namespace lexer;

void test()
{
        std::vector<lexer::token> tokens;
        std::string s1 = "create tbl (id integer, cnt integer)";
        std::string s2 = "insert into tbl (id, cnt) values ('1', '12')";
        std::string s3 = "insert into tbl (id, cnt) values ('5', '3')";
        std::string s4 = "insert into tbl (id, cnt) values ('2', '6')";
        std::string s5 = "select id from tbl where cnt < '7' " \
        "intersect select id from tbl where cnt > '4'";

        execute(s1);
        execute(s2);
        execute(s3);
        execute(s4);
        auto cols = execute(s5);
        if (!cols.empty()) {
                std::string res = cols.at(0).values.at(0)->GetValue();
                if (res == "2")
                        std::cout << "ok" << std::endl;
        } else
                std::cout << "not ok" << std::endl;

}

int main()
{
        test();
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
                        if (root) {
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
                                
                        }
                        
               }
                
        }
}