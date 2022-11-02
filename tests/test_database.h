#include <gtest/gtest.h>
#include "lexer.h"
#include "database.h"
#include "parser.h"

using namespace lexer;

std::vector<Column> execute(std::string str)
{
        Lexer lex;
        std::vector<lexer::token> tokens;
        Storage st("testdb");
        parser::Parser prs(std::make_shared<Storage>(st));
        std::vector<Column> cols;
        lexer::token tok(stmt::type::type_error, token::value(""));
        auto sit = str.begin();
        while (sit != str.end()) {
                sit = lex.getToken(sit, str.end(), tok);
                if (sit == str.end()) {
                        std::cout << "syntax error" << std::endl;
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
                } catch (std::exception &ex) {
                }
                
        }
        return cols;
}

void test()
{
        

}

TEST(TestDatabase, IntersectSelect) {
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
        ASSERT_TRUE(!cols.empty());
        ASSERT_TRUE(!cols.at(0).values.empty());
        EXPECT_EQ(cols.at(0).values.at(0)->GetValue(), "2");
}