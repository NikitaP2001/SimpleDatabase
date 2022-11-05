#include <algorithm>
#include <set>

#include "error.hpp"
#include "lexer.h"

using namespace lexer;

operator_map_t global::g_cOperatorMap = {
        { "=", operator_t::equal },
        { "!=", operator_t::not_equal },
        { ">", operator_t::greater },
        { "<", operator_t::less },
        { "<=", operator_t::less_equal },
        { ">=", operator_t::greater_equal }
};

lexer::separator_map_t global::g_cSeparatorMap = {
        { ",", separator_t::colon },
        { "(", separator_t::left_bracket },
        { ")", separator_t::right_bracket },
};

lexer::keyword_map_t global::g_cKeywordMap = {
        { "CREATE", keyword_t::create },
        { "DATE", keyword_t::date },
        { "DATELNVL", keyword_t::datelnvl },
        { "DROP", keyword_t::drop },
        { "FROM", keyword_t::from },
        { "IN", keyword_t::in },
        { "INTEGER", keyword_t::integer },
        { "DELETE", keyword_t::del },
        { "INTERSECT", keyword_t::intersect },
        { "INSERT", keyword_t::insert },
        { "INTO", keyword_t::into },
        { "REAL", keyword_t::real },
        { "SELECT", keyword_t::select },
        { "STRING", keyword_t::string },
        { "CHAR", keyword_t::chr },
        { "VALUES", keyword_t::values },
        { "WHERE", keyword_t::where }
};

static std::string::iterator getWord(std::string::iterator first, 
std::string::iterator last)
{
        std::string::iterator wordEnd = last;
        if (first != last && std::isalpha(*first)) {
                while (first != last && std::isalpha(*first))
                        std::advance(first, 1);
                wordEnd = std::prev(first);
        }
        return wordEnd;
}

static std::string::iterator getOperator(std::string::iterator first, 
std::string::iterator last)
{
        std::string::iterator wordEnd = last;
        std::set<char> operatorSym { '=', '<', '>', '!' };
        if (first != last && operatorSym.count(*(wordEnd = first++))) {
                if (first != last && operatorSym.count(*first)) {
                        wordEnd = first;
                }
        }
        return wordEnd;
}

static std::string::iterator getLiteral(std::string::iterator first, 
std::string::iterator last)
{
        std::string::iterator wordEnd = last;
        if (first != last && *first == '\'') {
                std::advance(first, 1);
                first = std::find(first, last, '\'');
                if (first != last)
                        wordEnd = first;
        }
        return wordEnd;
}

#include <iostream>
std::string::iterator lexer::Lexer::getToken(std::string::iterator first,
std::string::iterator last, token &firstToken)
{
        std::string::iterator tokPos = last;

        while (first != last && isspace(*first))
                std::advance(first, 1);
        if (first == last)
                return std::prev(first);
        if ((tokPos = getWord(first, last)) != last) {
                std::string token(first, tokPos + 1);
                std::transform(token.begin(), token.end(), token.begin(), ::toupper);
                if (global::g_cKeywordMap.count(token)) {
                        firstToken = lexer::token(stmt::type::keyword, 
                        token::value(global::g_cKeywordMap[token]));
                        INFO("keyword: " << token);
                } else {
                        firstToken = lexer::token(stmt::type::identifier, 
                        token::value(token));
                        INFO("identifier: " << token);
                }
        } else if ((tokPos = getLiteral(first, last)) != last) {
                firstToken.m_type = token::type::literal;
                std::string token(first + 1, tokPos);
                firstToken.m_value.m_literalValue = token;
                firstToken = lexer::token(stmt::type::literal,
                token::value(token));
                INFO("literal: " << token);
        } else if (global::g_cSeparatorMap.count(std::string(1, *first))) {
                firstToken = lexer::token(stmt::type::separator,
                token::value(global::g_cSeparatorMap[std::string(1, *first)]));
                tokPos = first;
                INFO("separator: " << std::string(1, *first));
        } else if ((tokPos = getOperator(first, last)) != last) {
                std::string token(first, tokPos + 1);
                if (global::g_cOperatorMap.count(token)) {
                        firstToken = lexer::token(stmt::type::op,
                        token::value(global::g_cOperatorMap[token]));
                        INFO("operator: " << token);
                } else {
                        tokPos = last;
                        ERR("unknown token: " << token);
                }
        } else {
                firstToken = lexer::token(stmt::type::type_error, 
                token::value(""));
                ERR("error, unrecognized symbol:" << *first);
                tokPos = last;
        }
        return tokPos;
}

stmt::type token::getType()
{
        return m_type;
}

token::value token::getValue()
{
        return m_value;
}

token::token(type tokenType, value tokenValue)
        : m_type(tokenType), m_value(tokenValue)
{

}


token::value::value(std::string val)
        : m_literalValue(val)
{

}


token::value::value(operator_t val)
        : m_operatorValue(val)
{

}


token::value::value(separator_t val)
        : m_separatorValue(val)
{

}


token::value::value(keyword_t val)
        : m_keywordValue(val)
{

}

std::string token::value::toString()
{
        return m_literalValue;
}

std::shared_ptr<DbValue> token::value::ToValue()
{
        return getDbValue(m_literalValue);
}

operator_t token::value::toOperator()
{
        return m_operatorValue;
}


keyword_t token::value::toKeyword()
{
        return m_keywordValue;
}


separator_t token::value::toSeparator()
{
        return m_separatorValue;
}


bool token::value::operator==(const value &other)
{
        return m_literalValue == other.m_literalValue
        && m_keywordValue == other.m_keywordValue
        && m_operatorValue == other.m_operatorValue
        && m_separatorValue == other.m_separatorValue;
}