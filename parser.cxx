#include "error.hpp"
#include "DbValue.h"
#include "parser.h"

using namespace parser;


stmt::type non_terminal::getType()
{
        return m_type;
}

non_terminal::non_terminal(type termType)
        : m_type(termType)
{

}

production::production(non_terminal src)
        : source(src)
{

}

void production::pushDest(non_terminal newDest)
{
        std::unique_ptr<stmt> ptrDest(new non_terminal(newDest));
        destination.push_back(std::move(ptrDest));
}

void production::pushDest(lexer::token newDest)
{
        std::unique_ptr<stmt> ptrDest(new lexer::token(newDest));
        destination.push_back(std::move(ptrDest));
}

Parser::Parser(std::shared_ptr<Storage> storage)
        : m_storage(storage)
{
// <query> = <selection> | <deletion> | <insertion> | <creation> | <drop>
        production queryToSelection(stmt::type::query);
        queryToSelection.pushDest(stmt::type::selection);
        m_producitons.push_back(std::move(queryToSelection));

        production queryToDeletion(stmt::type::query);
        queryToDeletion.pushDest(stmt::type::deletion);
        m_producitons.push_back(std::move(queryToDeletion));

        production queryToInsertion(stmt::type::query);
        queryToInsertion.pushDest(stmt::type::insertion);
        m_producitons.push_back(std::move(queryToInsertion));

        production queryToCreation(stmt::type::query);
        queryToCreation.pushDest(stmt::type::creation);
        m_producitons.push_back(std::move(queryToCreation));

        production queryToDrop(stmt::type::query);
        queryToDrop.pushDest(stmt::type::drop);
        m_producitons.push_back(std::move(queryToDrop));

// <selection> = <select-query> | <select-query> INTERSECT <selection>

        production selectAndIntercept(stmt::type::selection);
        selectAndIntercept.pushDest(stmt::type::select_query);
        selectAndIntercept.pushDest(lexer::token(stmt::type::keyword, 
        lexer::keyword_t::intersect));
        selectAndIntercept.pushDest(stmt::type::selection);
        m_producitons.push_back(std::move(selectAndIntercept));

        production selectionToSelect(stmt::type::selection);
        selectionToSelect.pushDest(stmt::type::select_query);
        m_producitons.push_back(std::move(selectionToSelect));

// <select-query> = SELECT <col-names> FROM <table-name> 
        // | SELECT <col-names> FROM <table-name> WHERE <expression>
        
        production selectWhere(stmt::type::select_query);
        selectWhere.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::select));
        selectWhere.pushDest(stmt::type::col_names);
        selectWhere.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::from));
        selectWhere.pushDest(stmt::type::table_name);
        selectWhere.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::where));
        selectWhere.pushDest(stmt::type::expression);
        m_producitons.push_back(std::move(selectWhere));

        production selectFrom(stmt::type::select_query);
        selectFrom.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::select));
        selectFrom.pushDest(stmt::type::col_names);
        selectFrom.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::from));
        selectFrom.pushDest(stmt::type::table_name);
        m_producitons.push_back(std::move(selectFrom));

// <expression> = <primary-expression> <operator> <primary-expression>
        // | <identifier> IN ( <selection> ) | <identifier> IN (<values>)
        production expressionOp(stmt::type::expression);
        expressionOp.pushDest(stmt::type::primary_expression);
        expressionOp.pushDest(stmt::type::op);
        expressionOp.pushDest(stmt::type::primary_expression);
        m_producitons.push_back(std::move(expressionOp));

        production inSelection(stmt::type::expression);
        inSelection.pushDest(stmt::type::identifier);
        inSelection.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::in));
        inSelection.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::left_bracket));
        inSelection.pushDest(stmt::type::selection);
        inSelection.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::right_bracket));
        m_producitons.push_back(std::move(inSelection));

        production inValues(stmt::type::expression);
        inValues.pushDest(stmt::type::identifier);
        inValues.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::in));
        inValues.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::left_bracket));
        inValues.pushDest(stmt::type::values);
        inValues.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::right_bracket));
        m_producitons.push_back(std::move(inValues));

// <deletion> = DELETE FROM <table-name>
        // | DELETE FROM <table-name> WHERE <expression>

        production deleteWhere(stmt::type::deletion);
        deleteWhere.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::del));
        deleteWhere.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::from));
        deleteWhere.pushDest(stmt::type::table_name);
        deleteWhere.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::where));
        deleteWhere.pushDest(stmt::type::expression);
        m_producitons.push_back(std::move(deleteWhere));

        production deleteFrom(stmt::type::deletion);
        deleteFrom.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::del));
        deleteFrom.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::from));
        deleteFrom.pushDest(stmt::type::table_name);
        m_producitons.push_back(std::move(deleteFrom));

// <drop> = DROP <table-name>

        production dropTable(stmt::type::drop);
        dropTable.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::drop));
        dropTable.pushDest(stmt::type::table_name);
        m_producitons.push_back(std::move(dropTable));

// <creation> = CREATE <table-name> (<column-definitions>)

        production createTable(stmt::type::creation);
        createTable.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::create));
        createTable.pushDest(stmt::type::table_name);
        createTable.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::left_bracket));
        createTable.pushDest(stmt::type::column_definitions);
        createTable.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::right_bracket));
        m_producitons.push_back(std::move(createTable));

// <column-definitions> = <identifier> <type> | <identifier> <type> , <column-definitions>

        production columnDefinitions(stmt::type::column_definitions);
        columnDefinitions.pushDest(stmt::type::identifier);
        columnDefinitions.pushDest(stmt::type::type);
        columnDefinitions.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::colon));
        columnDefinitions.pushDest(stmt::type::column_definitions);
        m_producitons.push_back(std::move(columnDefinitions));

        production columnDefinition(stmt::type::column_definitions);
        columnDefinition.pushDest(stmt::type::identifier);
        columnDefinition.pushDest(stmt::type::type);
        m_producitons.push_back(std::move(columnDefinition));

// <type> = string | integer | real | char | date | datelnvl

        production typeString(stmt::type::type);
        typeString.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::string));
        m_producitons.push_back(std::move(typeString));

        production typeInteger(stmt::type::type);
        typeInteger.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::integer));
        m_producitons.push_back(std::move(typeInteger));

        production typeReal(stmt::type::type);
        typeReal.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::real));
        m_producitons.push_back(std::move(typeReal));

        production typeChar(stmt::type::type);
        typeChar.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::chr));
        m_producitons.push_back(std::move(typeChar));

        production typeDate(stmt::type::type);
        typeDate.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::date));
        m_producitons.push_back(std::move(typeDate));

        production typeLnvl(stmt::type::type);
        typeLnvl.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::datelnvl));
        m_producitons.push_back(std::move(typeLnvl));

// <insertion> = INSERT INTO <table-name> (<col-names>) VALUES (<values>)

        production insertInto(stmt::type::insertion);
        insertInto.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::insert));
        insertInto.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::into));
        insertInto.pushDest(stmt::type::table_name);
        insertInto.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::left_bracket));
        insertInto.pushDest(stmt::type::col_names);
        insertInto.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::right_bracket));
        insertInto.pushDest(lexer::token(stmt::type::keyword,
        lexer::keyword_t::values));
        insertInto.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::left_bracket));
        insertInto.pushDest(stmt::type::values);
        insertInto.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::right_bracket));
        m_producitons.push_back(std::move(insertInto));

// <primary-expression> = <identifier> | <literal>

        production primaryToIdentifier(stmt::type::primary_expression);
        primaryToIdentifier.pushDest(stmt::type::identifier);
        m_producitons.push_back(std::move(primaryToIdentifier));

        production primaryToLiteral(stmt::type::primary_expression);
        primaryToLiteral.pushDest(stmt::type::literal);
        m_producitons.push_back(std::move(primaryToLiteral));

//<col-names> = <identifier> | <identifier> , <col-names>

        production colnamesToIds(stmt::type::col_names);
        colnamesToIds.pushDest(stmt::type::identifier);
        colnamesToIds.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::colon));
        colnamesToIds.pushDest(stmt::type::col_names);
        m_producitons.push_back(std::move(colnamesToIds));

        production colnamesToId(stmt::type::col_names);
        colnamesToId.pushDest(stmt::type::identifier);
        m_producitons.push_back(std::move(colnamesToId));

// <table-name> = <identifier>

        production tableNameToId(stmt::type::table_name);
        tableNameToId.pushDest(stmt::type::identifier);
        m_producitons.push_back(std::move(tableNameToId));

// <values> = <literal> | <literal>, <values>

        production valuesToLiterals(stmt::type::values);
        valuesToLiterals.pushDest(stmt::type::literal);
        valuesToLiterals.pushDest(lexer::token(stmt::type::separator,
        lexer::separator_t::colon));
        valuesToLiterals.pushDest(stmt::type::values);
        m_producitons.push_back(std::move(valuesToLiterals));

        production valuesToLiteral(stmt::type::values);
        valuesToLiteral.pushDest(stmt::type::literal);
        m_producitons.push_back(std::move(valuesToLiteral));
}

std::unique_ptr<database::Query> Parser::stmtToQuery(non_terminal source)
{
        std::unique_ptr<database::Query> res;
        switch (source.getType()) {
        case stmt::type::query:
                res = std::make_unique<database::Query>(m_storage);
                break;
        case stmt::type::selection:
                res = std::make_unique<database::Selection>(m_storage);
                break;
        case stmt::type::deletion:
                res = std::make_unique<database::Deletion>(m_storage);
                break;
        case stmt::type::insertion:
                res = std::make_unique<database::Insertion>(m_storage);
                break;
        case stmt::type::creation:
                res = std::make_unique<database::Creation>(m_storage);
                break;
        case stmt::type::drop:
                res = std::make_unique<database::Drop>(m_storage);
                break;
        case stmt::type::select_query:
                res = std::make_unique<database::SelectQuery>(m_storage);
                break;
        case stmt::type::col_names:
                res = std::make_unique<database::ColNames>(m_storage);
                break;
        case stmt::type::table_name:
                res = std::make_unique<database::TableName>(m_storage);
                break;
        case stmt::type::expression:
                res = std::make_unique<database::Expression>(m_storage);
                break;
        case stmt::type::primary_expression:
                res = std::make_unique<database::PrimaryExpression>(m_storage);
                break;
        case stmt::type::values:
                res = std::make_unique<database::Values>(m_storage);
                break;
        case stmt::type::column_definitions:
                res = std::make_unique<database::ColumnDefinitions>(m_storage);
                break;
        case stmt::type::type:
                res = std::make_unique<database::Type>(m_storage);
                break;
        case stmt::type::identifier:
                res = std::make_unique<database::Identifier>(m_storage);
                break;
        case stmt::type::literal:
                res = std::make_unique<database::Literal>(m_storage);
                break;
        }
        return std::move(res); 
}

std::unique_ptr<database::Query> Parser::tryProduction(
        std::vector<lexer::token>::iterator &first,
        std::vector<lexer::token>::iterator last,
        production &prod)
{
        auto q = stmtToQuery(prod.source);
        auto it = first;
        for (auto &st : prod.destination) {

                if (st->getType() == stmt::type::keyword && first != last) {
                        lexer::token *tok = nullptr;
                        if (tok = dynamic_cast<lexer::token*>(
                                st.get())) {
                                if (first->getValue() == tok->getValue()) {
                                        q->setStmt(std::make_shared<lexer::token>(
                                                *first));
                                        first++;
                                        continue;
                                } 
                        }
                }
                if (st->getType() == stmt::type::separator && first != last) {
                        lexer::token *tok = nullptr;
                        if (tok = dynamic_cast<lexer::token*>(
                                st.get())) {
                                if (first->getValue() == tok->getValue()) {
                                        first++;
                                        continue;
                                }
                        }
                }
                
                if (first != last) {
                        auto subQuery = getTree(st->getType(), first, last);
                        if (subQuery) {
                                q->pushQuery(std::move(subQuery));
                                continue;
                        }
                }
                q.reset();
                first = it;
                break;
        }
        return q;
}

std::unique_ptr<database::Query> Parser::getTree(stmt::type source, 
std::vector<lexer::token>::iterator &tokFirst, 
std::vector<lexer::token>::iterator tokLast)
{
        if (source == stmt::type::identifier && tokFirst != tokLast) {
                if (tokFirst->getType() == source) {
                        auto q = std::make_unique<database::Identifier>(m_storage);
                        q->setStmt(std::make_shared<lexer::token>(*tokFirst));
                        tokFirst++;
                        return q;
                } else
                        return std::unique_ptr<database::Query>{};
        }
        if (source == stmt::type::literal && tokFirst != tokLast) {
                if (tokFirst->getType() == source) {
                        auto q = std::make_unique<database::Literal>(m_storage);
                        q->setStmt(std::make_shared<lexer::token>(*tokFirst));
                        tokFirst++;
                        return q;
                } else
                        return std::unique_ptr<database::Query>{};
        }
        if (source == stmt::type::op && tokFirst != tokLast) {
                if (tokFirst->getType() == source) {
                        auto q = std::make_unique<database::Operator>(m_storage);
                        q->setStmt(std::make_shared<lexer::token>(*tokFirst));
                        tokFirst++;
                        return q;
                } else
                        return std::unique_ptr<database::Query>{};
        }

        for (auto &prod : m_producitons) {
                if (prod.source.getType() == source) {
                        auto it = tokFirst;
                        auto q = tryProduction(tokFirst, tokLast, prod);
                        if (q)
                                return q;
                        else
                                tokFirst = it;
                }
        }
        return std::unique_ptr<database::Query>{};
}