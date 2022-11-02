#include <gtest/gtest.h>
#include "lexer.h"


using namespace lexer;

TEST(TestLexer, TestSelectIn) {
        Lexer lex;
        std::string testInput = "SELECT FROM file where"\
        " ID IN('1', '12312', '11')";

        std::vector<token> tokens = {
                lexer::token(stmt::type::keyword, token::value(keyword_t::select)),
                lexer::token(stmt::type::keyword, token::value(keyword_t::from)),
                lexer::token(stmt::type::identifier, token::value("FILE")),
                lexer::token(stmt::type::keyword, token::value(keyword_t::where)),
                lexer::token(stmt::type::identifier, token::value("ID")),
                lexer::token(stmt::type::keyword, token::value(keyword_t::in)),
                lexer::token(stmt::type::separator, token::value(separator_t::left_bracket)),
                lexer::token(stmt::type::literal, token::value("1")),
                lexer::token(stmt::type::separator, token::value(separator_t::colon)),
                lexer::token(stmt::type::literal, token::value("12312")),
                lexer::token(stmt::type::separator, token::value(separator_t::colon)),
                lexer::token(stmt::type::literal, token::value("11")),
                lexer::token(stmt::type::separator, token::value(separator_t::right_bracket))
        };
        auto itext = testInput.begin();
        auto iToks = tokens.begin();
        while (itext != testInput.end() && iToks != tokens.end()) {
                lexer::token tok(stmt::type::type_error, token::value(""));
                if ((itext = lex.getToken(itext, testInput.end(), 
                tok)) != testInput.end()) {
                    EXPECT_TRUE((*iToks).getType() == tok.getType() && 
                    (*iToks).getValue() == tok.getValue());
                    itext++;    
                    iToks++;
                }
                
        }
        ASSERT_TRUE(itext == testInput.end());
}