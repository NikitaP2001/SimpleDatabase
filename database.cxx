#include <algorithm>
#include <set>

#include "error.hpp"
#include "lexer.h"
#include "database.h"

using namespace database;

bool Response::getStmt(std::shared_ptr<stmt> &statement)
{
        bool status = false;
        if (m_statement) {
                statement = m_statement;
                status = true;
        }
        return status;
}

Response::Response(std::vector<Column> cols)
{
        m_columns.insert(m_columns.end(), cols.begin(), cols.end());
}

Response::Response(std::shared_ptr<stmt> st)
{
        m_statement = st;
}


void Response::pushColumn(Column &col)
{
        m_columns.push_back(std::move(col));
}


void Response::setStmt(std::shared_ptr<stmt> statement)
{
        m_statement = statement; 
}


void Response::pushDef(Storage::ColumnType def)
{ 
        m_defs.push_back(def);
}


Storage::ColumnType Response::popDef()
{
        return m_defs.front(); m_defs.pop_back();
}


std::vector<Storage::ColumnType> Response::popDefs()
{
        return std::move(m_defs);
}


void Response::pushDefs(std::vector<Storage::ColumnType> &&defs)
{ 
        m_defs.insert(m_defs.end(), std::make_move_iterator(defs.begin()),
        std::make_move_iterator(defs.end()));
}


bool Response::getColumn(OUT Column &result)
{
        bool status = false;
        if (m_columns.size() != 0) {
                result = std::move(m_columns.front());
                m_columns.pop_front();
                status = true;
        }
        return status;
}

std::shared_ptr<Storage> Query::getStorage()
{
        return m_storage;
}

Query::Query(std::shared_ptr<Storage> storage)
        : m_storage(storage) {};

void Query::pushQuery(std::unique_ptr<Query> query)
{
        m_queries.push_back(std::move(query));
}

void Query::popQuery(std::vector<Query*> &queries)
{
        for (auto &q : m_queries)
                queries.push_back(q.get());
}


Response Query::execute(std::string tableName)
{
        INFO("executing query");
        Response res;
        if (m_queries.size() == 1)
                res = m_queries[0]->execute("");
        return std::move(res);
}

Type::Type(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}

Response Type::execute(std::string tableName)
{
        INFO("executing type");
        Response ret;

        std::shared_ptr<stmt> st;
        if (st = getStmt()) {
                ret.setStmt(st);
        } else {
                ERR("Empty type token");
        }
        return ret;
        return Response();
}

Selection::Selection(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}


static bool cmpRow(std::vector<Column> &colLeft, uint32_t lRow,
        std::vector<Column> &colRigth, uint32_t rRow)
{
        bool result = true;
        for (auto &col : colLeft) {
                auto itSameCol = std::find_if(colRigth.begin(), colRigth.end(),
                [&](auto &thisCol) { return thisCol.name == col.name; });

                if (itSameCol == colRigth.end()) {
                        result = false;
                        break;
                }

                if (!(*(col.values[lRow].get()) == itSameCol->values[rRow].get()))
                {
                        result = false;
                        break;
                }
        }
        return result;
}


static void intersect(std::vector<Column> &colLeft, std::vector<Column> &colRigth)
{
        for (uint32_t i = 0; i < colLeft.at(0).values.size(); ) {
                bool inBoth = false;
                for (uint32_t j = 0; j < colRigth.at(0).values.size(); j++) {
                        if (cmpRow(colLeft, i, colRigth, j))
                                inBoth = true; 
                }
                if (!inBoth) {
                        for (auto &col : colLeft)
                                col.values.erase(col.values.begin() + i);
                } else {
                        i += 1;
                }
        }
}

Response Selection::execute(std::string tableName)
{
        INFO("executing selection");
        std::vector<Query*> chldQueries;
        Response res;
        Column col;
        Query *q1 = nullptr, *q2 = nullptr;
        popQuery(chldQueries);

        if (chldQueries.size() >= 1) {
                if (!(q1 = dynamic_cast<SelectQuery*>(chldQueries[0]))) {
                        ERR("wrong select query format");
                        return res;
                }
                std::vector<Column> cols;
                Response lColsRes = q1->execute("");
                while (lColsRes.getColumn(col))
                        cols.push_back(col);

                if (chldQueries.size() == 2) {
                        if (!(q2 = dynamic_cast<Selection*>(chldQueries[1]))) {
                                ERR("wrong select query format");
                                return res;
                        }       

                        std::vector<Column> otherCols;
                        Response subRes = q2->execute("");
                        while (subRes.getColumn(col))
                                otherCols.push_back(col);
                        if (otherCols.size() == cols.size() && cols.size() != 0) {
                                intersect(cols, otherCols);
                                
                        } else
                                throw std::logic_error("invalid query");
                }

                for (auto &col : cols)
                        res.pushColumn(col);
        }

        return res;
}

SelectQuery::SelectQuery(std::shared_ptr<Storage> storage) 
        : Query(storage)
{
}

Response SelectQuery::execute(std::string tableName)
{
        INFO("executing select query");
        Query *q1 = nullptr, *q2 = nullptr, *q3 = nullptr;
        std::vector<Query*> chldQueries;
        Column col;
        Response res;
        popQuery(chldQueries);

        if (chldQueries.size() >= 2) {
                if (!((q1 = dynamic_cast<ColNames*>(chldQueries[0]))
                && (q2 = dynamic_cast<TableName*>(chldQueries[1])))) {
                        ERR("wrong select query format");
                        return Response();
                }
        }

        if (chldQueries.size() == 3) {
                if (!(q3 = dynamic_cast<Expression*>(chldQueries[2]))) {
                        ERR("wrong select query format");
                        return Response();
                }
        }

        std::shared_ptr<stmt> stName;
        lexer::token *nameTok;
        if (q2->execute("").getStmt(stName)
        && (nameTok = dynamic_cast<lexer::token*>(stName.get()))) {
                tableName = nameTok->getValue().toString();
        } else {
                ERR("wrong select: name empty");
                return Response();
        }

        std::set<std::string> colNames;
        Response colNamesRes = q1->execute("");
        while (colNamesRes.getColumn(col))
                colNames.insert(col.name);

        if (q3) {
                Response expr = q3->execute(tableName);        
                while (expr.getColumn(col)) {
                        if (colNames.count(col.name) != 0)
                                res.pushColumn(col);
                }
        } else {
                std::vector<Column> allCols = getStorage()->getCols(tableName);
                for (auto &col : allCols) {
                        if (colNames.count(col.name) != 0)
                                res.pushColumn(col);
                }
        }

        return res;
}

Expression::Expression(std::shared_ptr<Storage> storage)
        : Query(storage)
{

};

Response Expression::execute(std::string tblName)
{
        Response ret;
        INFO("executing expression");
        std::vector<Query*> chldQueries;
        popQuery(chldQueries);
        if (chldQueries.size() >= 2) {
                Query *q1 = nullptr, *q2 = nullptr, *q3 = nullptr;
                if ((q1 = dynamic_cast<PrimaryExpression*>(chldQueries[0]))
                && (q2 = dynamic_cast<Operator*>(chldQueries[1]))
                && (q3 = dynamic_cast<PrimaryExpression*>(chldQueries[2]))) {
                        std::shared_ptr<stmt> stmt1, stmt2, stmt3;
                        if (q1->execute("").getStmt(stmt1) 
                        && q2->execute("").getStmt(stmt2)
                        && q3->execute("").getStmt(stmt3)) {
                                std::vector<Column> result 
                                = getStorage()->getByExpr(tblName,
                                stmt1, stmt2, stmt3);
                                if (!result.empty()) {
                                        ret = Response(result);
                                } else {
                                        INFO("Storage query empty");
                                }
                        } else {
                                ERR("invalid expression format");
                        }

                } else if ((q1 = dynamic_cast<Identifier*>(chldQueries[0]))
                && (q2 = dynamic_cast<Selection*>(chldQueries[1]))) {
                        throw std::runtime_error("not supported");
                } else if ((q1 = dynamic_cast<Identifier*>(chldQueries[0]))
                && (q2 = dynamic_cast<Values*>(chldQueries[1]))) {
                        throw std::runtime_error("not supported");
                } else {
                        ERR("invalid query format");
                }
        } else {
                ERR("invalid size");
        }
        return ret;
}

Deletion::Deletion(std::shared_ptr<Storage> storage)
        : Query(storage)
{
        
}

Response Deletion::execute(std::string tableName)
{
        INFO("executing deletion");
        Query *q1 = nullptr, *q2 = nullptr;
        std::vector<Query*> chldQueries;
        Column col;
        Response res;
        popQuery(chldQueries);

        if (chldQueries.size() >= 1) {
                std::shared_ptr<stmt> stName;
                lexer::token *nameTok;
                if (chldQueries[0]->execute("").getStmt(stName)
                && (nameTok = dynamic_cast<lexer::token*>(stName.get()))) {
                        tableName = nameTok->getValue().toString();
                } else
                        return res;
        }

        std::vector<Column> cols;
        if (chldQueries.size() >= 2) {
                std::vector<Column> otherCols;
                Response expr = chldQueries[1]->execute(tableName);        
                while (expr.getColumn(col))
                        otherCols.push_back(col);
                cols = getStorage()->getCols(tableName);
                if (!cols[0].values.empty() && !otherCols[0].values.empty()) {
                        for (int i = 0; i < cols[0].values.size(); ) {
                                bool present = false;
                                for (int j = 0; j < otherCols[0].values.size(); j++) {
                                        if (*(cols[0].values[i].get()) 
                                        == otherCols[0].values[j].get()) {
                                                present = true;
                                                break;
                                        }
                                        
                                }
                                if (present) {
                                        for (auto &col : cols)
                                                col.values.erase(
                                                        col.values.begin() + i);
                                } else {
                                        i++;
                                }
                        }
                }
        }
        if (!getStorage()->writeValues(cols, tableName))
                throw std::runtime_error("delete failed");

        return res;
}

Drop::Drop(std::shared_ptr<Storage> storage)
        : Query(storage)
{
        
}

Response Drop::execute(std::string tableName)
{
        INFO("executing drop");
        std::vector<Query*> chldQueries;
        Query *q = nullptr;
        popQuery(chldQueries);

        if (chldQueries.size() == 1) {
                std::shared_ptr<stmt> stName;
                lexer::token *nameTok;
                if (chldQueries[0]->execute("").getStmt(stName)
                && (nameTok = dynamic_cast<lexer::token*>(stName.get()))) {
                        tableName = nameTok->getValue().toString();
                        getStorage()->dropTable(tableName);
                } else {
                        ERR("wrong select: name empty");
                        return Response();
                }
        } else {
                ERR("wring delete size");
        }

        return Response();
}

Creation::Creation(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}

Response Creation::execute(std::string tableName)
{
        INFO("executing creation");
        Response res;
        TableName *q1;
        ColumnDefinitions *q2;
        std::vector<Query*> chldQueries;
        popQuery(chldQueries);
        if (chldQueries.size() == 2) {
                if ((q1 = dynamic_cast<TableName*>(chldQueries[0]))
                && (q2 = dynamic_cast<ColumnDefinitions*>(chldQueries[1]))) {
                        std::shared_ptr<stmt> tableName;
                        q1->execute("").getStmt(tableName);
                        if (!tableName) {
                                ERR("failed get table name");
                                return std::move(res);
                        }
                        auto colDefs = q2->execute("").popDefs();
                        if (colDefs.empty()) {
                                ERR("failed get col defs");
                                return std::move(res);
                        }
                        if (!getStorage()->createTable(tableName.get(), colDefs))
                                throw std::runtime_error(
                                        "invalid create query");
                }
        } else {
                ERR("invalid create format");
        }

       
        return std::move(res);
}

ColumnDefinitions::ColumnDefinitions(std::shared_ptr<Storage> storage)
        : Query(storage)
{
        
}

Response ColumnDefinitions::execute(std::string tableName)
{
        INFO("executing column definition");
        Response res;
        ColumnDefinitions *q3 = nullptr;
        Identifier *q1;
        Type *q2;
        std::vector<Query*> chldQueries;
        popQuery(chldQueries);
        if (chldQueries.size() >= 2) {
                if ((q1 = dynamic_cast<Identifier*>(chldQueries[0]))
                && (q2 = dynamic_cast<Type*>(chldQueries[1]))) {
                        std::shared_ptr<stmt> stmt1, stmt2;
                        if (q1->execute("").getStmt(stmt1) 
                        && q2->execute("").getStmt(stmt2)) {
                                Storage::ColumnType def;
                                if (stmtToColumnDef(stmt1.get(), 
                                stmt2.get(), def)) {
                                        res.pushDef(def);
                                } else {
                                        ERR("unable to get definition");
                                }
                        }
                } else {
                        ERR("Inlvalid query format");
                }
                if (chldQueries.size() == 3) {
                        if (q3 = dynamic_cast<ColumnDefinitions*>(
                                chldQueries[2])) {
                                res.pushDefs(q3->execute("").popDefs());
                        } else {
                                ERR("invalid col-def format");
                        }
                }
        }

        return std::move(res);
}

Insertion::Insertion(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}

Response Insertion::execute(std::string tableName)
{
        INFO("executing insertion");
        std::shared_ptr<stmt> stName;
        Query *q1 = nullptr, *q2 = nullptr, *q3 = nullptr;
        std::vector<Query*> chldQueries;
        Column col;
        popQuery(chldQueries);

        if (chldQueries.size() != 3) {
                ERR("wrong insertion format");
                return Response();
        }

        if (!(q1 = dynamic_cast<TableName*>(chldQueries[0]))) {
                ERR("insertion: no tbl name");
                return Response();
        }

        lexer::token *nameTok = nullptr;
        if (q1->execute("").getStmt(stName)
        && (nameTok = dynamic_cast<lexer::token*>(stName.get()))) {
                tableName = nameTok->getValue().toString();
        } else {
                ERR("wrong select: name empty");
                return Response();
        }

        if (!(q2 = dynamic_cast<ColNames*>(chldQueries[1]))) {
                ERR("wrong select: name empty");
                return Response();
        }

        std::vector<std::string> colNames;
        Response colNamesRes = q2->execute("");
        while (colNamesRes.getColumn(col))
                colNames.push_back(col.name);

        if (!(q3 = dynamic_cast<Values*>(chldQueries[2]))) {
                ERR("wrong select: values empty");
                return Response();
        }

        std::vector<Column> colValues;
        Response valueRes = q3->execute("");
        auto nameIt = colNames.begin();
        while (valueRes.getColumn(col)) {
                if (nameIt == colNames.end())
                        break;
                col.name = *nameIt++;
                colValues.push_back(std::move(col));
        }

        if (colValues.size() == colNames.size() && !colNames.empty()) {
                if (!getStorage()->insertValues(colValues, tableName))
                        throw std::runtime_error("failed to insert");
        } else {
                throw std::logic_error("invalid query size");
        }
                
        

        return Response();
}

PrimaryExpression::PrimaryExpression(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}

Response PrimaryExpression::execute(std::string tableName)
{
        INFO("executing primary expression");
        Response ret;
        std::shared_ptr<stmt> st;
        std::vector<Query*> chldQueries;
        popQuery(chldQueries);
        if (chldQueries.size() == 1) {
                if (chldQueries[0]->execute("").getStmt(st)) {
                        ret.setStmt(st);
                } else {
                        ERR("Empty primary expr");
                }
        }
        
        return ret;
}

ColNames::ColNames(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}

Response ColNames::execute(std::string tableName)
{
        INFO("executing colnames");
        Response ret;
        std::vector<Query*> chldQueries;
        std::shared_ptr<stmt> st;
        Query *q1 = nullptr, *q2 = nullptr;
        lexer::token *tok = nullptr;
        popQuery(chldQueries);
        if (chldQueries.size() >= 1) {
                if (q1 = dynamic_cast<Identifier*>(chldQueries[0])) {
                        if (q1->execute("").getStmt(st)) {
                                if (tok = dynamic_cast<lexer::token*>(st.get())) {
                                        Column col;
                                        col.name = tok->getValue().toString();
                                        ret.pushColumn(col);
                                }
                        }
                }
        } else {
                ERR("wrong column names format");
        }
        if (chldQueries.size() == 2) {
                if (q2 = dynamic_cast<ColNames*>(chldQueries[1])) {
                        Column col;
                        Response colRes = q2->execute("");
                        while (colRes.getColumn(col))
                                ret.pushColumn(col);
                }
        }

        return ret;
}

TableName::TableName(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}

Response TableName::execute(std::string tableName)
{
        INFO("executing table name");
        Response ret;
        std::shared_ptr<stmt> st;
        std::vector<Query*> chldQueries;
        popQuery(chldQueries);
        if (chldQueries.size() == 1) {
                if (st = chldQueries[0]->getStmt()) {
                        ret.setStmt(st);
                } else {
                        ERR("Empty name");
                }
        } else {
                ERR("inalid table name query");
        }
        
        return ret;
}

Values::Values(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}

Response Values::execute(std::string tableName)
{
        INFO("executing values");
        Response ret;
        std::vector<Query*> chldQueries;
        std::shared_ptr<stmt> st;
        Query *q1 = nullptr, *q2 = nullptr;
        lexer::token *tok = nullptr;
        popQuery(chldQueries);
        if (chldQueries.size() >= 1) {
                if (q1 = dynamic_cast<Literal*>(chldQueries[0])) {
                        if (q1->execute("").getStmt(st)) {
                                if (tok = dynamic_cast<lexer::token*>(st.get())) {
                                        Column col;
                                        col.values.push_back(getDbValue(
                                                tok->getValue().toString()));
                                        ret.pushColumn(col);
                                }
                        }
                }
        } else {
                ERR("wrong values format");
        }
        if (chldQueries.size() == 2) {
                if (q2 = dynamic_cast<Values*>(chldQueries[1])) {
                        Column col;
                        Response res = q2->execute("");
                        while (res.getColumn(col))
                                ret.pushColumn(col);
                }
        }

        return ret;
}

Operator::Operator(std::shared_ptr<Storage> storage)
        : Query(storage)
{

}

Response Operator::execute(std::string tableName)
{
        INFO("executing operator");
        Response ret;
        std::shared_ptr<stmt> st;
        if (st = getStmt()) {
                ret.setStmt(st);
        } else {
                ERR("Empty operator");
        }
        return ret;
}

Identifier::Identifier(std::shared_ptr<Storage> storage) 
        : Query(storage)
{

}

Response Identifier::execute(std::string tableName)
{
        INFO("executing identifier");
        Response ret;
        std::shared_ptr<stmt> st;

        if (st = getStmt()) {
                ret.setStmt(st);
        } else {
                ERR("Empty identifier");
        }
        return ret;
}

Literal::Literal(std::shared_ptr<Storage> storage) 
        : Query(storage)
{

}

Response Literal::execute(std::string tableName)
{
        INFO("executing literal");
        Response ret;
        std::shared_ptr<stmt> st;
        if (st = getStmt()) {
                ret.setStmt(st);
        } else {
                ERR("Empty literal");
        }
        return ret;
}