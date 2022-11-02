#pragma once

#include <memory>
#include <vector>

#include "statement.h"
#include "DbValue.h"

class Storage {

public:
        enum dataType : int {
                string_t,
                integer_t,
                real_t,
                char_t,
                date_t,
                datelnvl_t
        };

        struct ColumnType {
                dataType type;
                std::string name;
        };
public:
        Storage(std::string path);

        std::vector<Column> getByExpr(std::string tblName,
        std::shared_ptr<stmt> stmt1, 
        std::shared_ptr<stmt> stmt2,
        std::shared_ptr<stmt> stmt3);

        std::vector<Column> getCols(std::string tblName);

        bool createTable(stmt *identifier, std::vector<ColumnType> cols);

        bool dropTable(std::string tableName);

        bool insertValues(std::vector<Column> &cols, std::string tableName);

        bool writeValues(std::vector<Column> &cols, std::string tableName);

private:
        std::string m_dbPath;

        constexpr static int k_colNameMax = 100;
        constexpr static int m_kHeadersOffset = 200;

private:



#pragma pack(push, 1)

        struct TableHeader {
                uint32_t numEntries;
                // first col of first row
                uint32_t firstEntry; 
                uint64_t numberColumns;
        };

        struct TypeHeader {
                dataType type;
                uint64_t colNameLen;
        };
        

        struct ColumnHeader {
                uint32_t dataLen;
                uint32_t nextRow;
        };

        std::vector<ColumnType> readColumnTypes(std::fstream &fin, uint32_t count);
        std::unique_ptr<DbValue> createValue(const ColumnType &type);

        bool readRow(std::fstream &fin, std::vector<Column> &cols, 
        const std::vector<ColumnType> &types);

        uint32_t getRowEnd(std::fstream &fio, uint32_t colsCount);

        bool readTable(std::string tableName, std::vector<Column> &cols);

        uint32_t writeTable(std::fstream &fio, std::vector<Column> &cols);

#pragma pack(pop)

};

bool stmtToColumnDef(stmt *identifier, stmt *type, Storage::ColumnType &colDef);