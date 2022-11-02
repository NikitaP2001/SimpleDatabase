#include <gtest/gtest.h>
#include "lexer.h"
#include "parser.h"

using namespace lexer;
using namespace parser;

TEST(TestParser, TestCreate) {
        std::vector<token> tokens = {
                lexer::token(stmt::type::keyword, token::value(keyword_t::create)),
                lexer::token(stmt::type::identifier, token::value("File")),
                lexer::token(stmt::type::separator, token::value(
                        separator_t::left_bracket)),
                lexer::token(stmt::type::identifier, token::value("id")),
                lexer::token(stmt::type::keyword, token::value(keyword_t::string)),
                lexer::token(stmt::type::separator, token::value(
                        separator_t::right_bracket)),
        };
        Storage st("testdb");
        Parser prs(std::make_shared<Storage>(st));
        if (tokens.begin() != tokens.end()) {
                auto itTokBeg = tokens.begin();
                auto root = 
                prs.getTree(stmt::type::query, itTokBeg, tokens.end());
                ASSERT_TRUE(root != nullptr);
        }
}

TEST(TestParser, TestSelectWhere) {

        std::vector<token> tokens = {
                lexer::token(stmt::type::keyword, token::value(keyword_t::select)),
                lexer::token(stmt::type::identifier, token::value("ID")),
                lexer::token(stmt::type::keyword, token::value(keyword_t::from)),
                lexer::token(stmt::type::identifier, token::value("FILE")),
                lexer::token(stmt::type::keyword, token::value(keyword_t::where)),
                lexer::token(stmt::type::identifier, token::value("ID")),
                lexer::token(stmt::type::op, token::value(operator_t::less)),
                lexer::token(stmt::type::literal, token::value("100")),
        };
        Storage st("testdb");
        Parser prs(std::make_shared<Storage>(st));
        if (tokens.begin() != tokens.end()) {
                auto itTokBeg = tokens.begin();
                auto root = 
                prs.getTree(stmt::type::query, itTokBeg, tokens.end());
                ASSERT_TRUE(root != nullptr);
        }
}