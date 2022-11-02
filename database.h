#pragma once
#include <list>
#include <memory>
#include <vector>
#include <string>

#include "storage.h"

#define IN
#define OUT

namespace database {

class Response {
public:
        Response() = default;
        explicit Response(std::vector<Column> cols);
        explicit Response(std::shared_ptr<stmt> st);

        bool getColumn(OUT Column &result);
        void pushColumn(Column &col);

        void setStmt(std::shared_ptr<stmt> statement);
        bool getStmt(OUT std::shared_ptr<stmt> &statement);

        void pushDef(Storage::ColumnType def);
        Storage::ColumnType popDef();

        std::vector<Storage::ColumnType> popDefs();
        void pushDefs(std::vector<Storage::ColumnType> &&defs);

private:
        std::vector<Storage::ColumnType> m_defs;
        std::shared_ptr<stmt> m_statement;
        std::list<Column> m_columns;
};

class Query {

public:
        Query(std::shared_ptr<Storage> storage);

        void popQuery(std::vector<Query*> &queries);
        void pushQuery(std::unique_ptr<Query> query);

        void setStmt(std::shared_ptr<stmt> statement)
        { m_statement = statement; }
        std::shared_ptr<stmt> getStmt()
        { return m_statement; }
        virtual Response execute(std::string tableName);

protected:
        std::shared_ptr<Storage> getStorage();

private:
        std::shared_ptr<stmt> m_statement;
        std::shared_ptr<Storage> m_storage;
        std::vector<std::unique_ptr<Query>> m_queries;
                 
};

class Type : public Query {

public:
        Type(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
        std::vector<std::unique_ptr<Query>> m_queries;

};

class Selection : public Query {

public:
        Selection(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class SelectQuery : public Query {

public:
        SelectQuery(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
        std::vector<std::unique_ptr<Query>> m_queries;
};


class Expression : public Query {

public:
        Expression(std::shared_ptr<Storage> storage);

        // returns table id`s collumn
        Response execute(std::string tableName) override;
};

class Deletion : public Query {

public:
        Deletion(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class Drop : public Query {

public:
        Drop(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class Creation : public Query {

public:
        Creation(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class ColumnDefinitions : public Query {

public:
        ColumnDefinitions(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class Insertion : public Query {

public:
        Insertion(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class PrimaryExpression : public Query {

public:
        PrimaryExpression(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class ColNames : public Query {

public:
        ColNames(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class TableName : public Query {

public:
        TableName(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class Values : public Query {

public:
        Values(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class Operator : public Query {

public:
        Operator(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class Identifier : public Query {
public:
        Identifier(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

class Literal : public Query {
public:
        Literal(std::shared_ptr<Storage> storage);
        Response execute(std::string tableName) override;
};

}