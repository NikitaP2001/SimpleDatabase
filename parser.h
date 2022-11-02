#pragma once
#include <vector>
#include <utility>
#include <memory>

#include "database.h"
#include "lexer.h"

namespace parser {

        class non_terminal : public stmt {
        public:
                non_terminal(type termType);
                type getType() override;

        private:
                type m_type;
        };

        struct production {
                production(non_terminal src);
                void pushDest(non_terminal newDest);
                void pushDest(lexer::token newDest);

                non_terminal source;
                std::vector<std::unique_ptr<stmt>> destination;
        };

        

        class Parser {

        public:
                Parser(std::shared_ptr<Storage> storage);

                std::unique_ptr<database::Query> getTree(stmt::type source, 
                std::vector<lexer::token>::iterator &tokFirst, 
                std::vector<lexer::token>::iterator tokLast);


        private:
                std::unique_ptr<database::Query> stmtToQuery(non_terminal source);
                std::unique_ptr<database::Query> tryProduction(
                std::vector<lexer::token>::iterator &first,
                std::vector<lexer::token>::iterator last,
                production &prod);
        private:
                std::shared_ptr<Storage> m_storage;
                std::vector<production> m_producitons;
        };

}