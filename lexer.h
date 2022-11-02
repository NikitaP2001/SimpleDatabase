#pragma once
#include <memory>
#include <iterator>
#include <map>

#include "statement.h"
#include "DbValue.h"

#define IN
#define OUT



namespace lexer {

        enum class operator_t;
        using operator_map_t = std::map<std::string, operator_t>;

        enum class separator_t;
        using separator_map_t = std::map<std::string, separator_t>;

        enum class keyword_t;
        using keyword_map_t = std::map<std::string, keyword_t>;

        class token;

        enum class operator_t {
                invalid_value,
                equal,
                not_equal,
                greater,
                less,
                less_equal,
                greater_equal,
        };

        enum class separator_t {
                invalid_value,
                colon,
                left_bracket,
                right_bracket
        };

        enum class keyword_t {
                invalid_value,
                select,
                del,
                intersect,
                insert,
                from,
                where,
                in,
                drop,
                create,
                into,
                values,

                string, 
                integer,
                real, 
                chr,
                date,
                datelnvl
        };

        class Lexer {
        public:
                std::string::iterator getToken(IN std::string::iterator first,
                IN std::string::iterator last, OUT token &firstToken);
        private:
        };


        class token : public stmt {
        friend class Lexer;
        public:
                class value {
                friend class Lexer;
                public:
                        value(std::string val);
                        value(operator_t val);
                        value(separator_t val);
                        value(keyword_t val);
                        bool operator==(const value &other);

                        operator_t  toOperator();
                        separator_t toSeparator();
                        keyword_t   toKeyword();
                        std::string toString();
                        std::shared_ptr<DbValue> ToValue();
                private:
                        std::string m_literalValue = "";
                        operator_t m_operatorValue = operator_t::invalid_value;
                        separator_t m_separatorValue = separator_t::invalid_value;
                        keyword_t m_keywordValue = keyword_t::invalid_value;
                };

                token(type tokenType, value tokenValue);
                type getType() override;
                value getValue();
        private:
                type m_type;
                value m_value; 
        };

}

namespace global {

        extern lexer::operator_map_t g_cOperatorMap;

        extern lexer::separator_map_t g_cSeparatorMap;

        extern lexer::keyword_map_t g_cKeywordMap;

};