#include <sstream>
#include <functional>
#include <filesystem>
#include <fstream>

#include "error.hpp"
#include "parser.h"
#include "lexer.h"
#include "storage.h"

namespace fs = std::filesystem;

Storage::Storage(std::string path)
        : m_dbPath(path)
{
        fs::path dbPath = path;
        if (fs::exists(path)) {
                if (!fs::is_directory(path))
                        throw std::runtime_error("wrong db format");
        } else
                fs::create_directories(path);
}


std::vector<Storage::ColumnType> Storage::readColumnTypes(std::fstream &fin, 
uint32_t count)
{
        TypeHeader typeHdr {};
        std::vector<ColumnType> types;
        for (uint32_t i = 0; i < count; i++) {
                if (fin.read((char*)&typeHdr, sizeof(typeHdr))) {
                        auto buffer = std::make_unique<char[]>(typeHdr.colNameLen);
                        if (fin.read(buffer.get(), typeHdr.colNameLen))
                                types.push_back({ typeHdr.type, buffer.get() });
                        else
                                throw std::runtime_error("fail: read col name");
                } else {
                        throw std::runtime_error("fail: read type header ");
                }
        }
        return types;
}

std::unique_ptr<DbValue> Storage::createValue(const ColumnType &type)
{
        std::unique_ptr<DbValue> val;
        switch (type.type) {
                case string_t:
                        val = std::make_unique<DbString>(DbString());
                        break;
                case integer_t:
                        val = std::make_unique<DbInteger>(DbInteger());
                        break;
                case real_t:
                        val = std::make_unique<DbReal>(DbReal());
                        break;
                case char_t:
                        val = std::make_unique<DbChar>(DbChar());
                        break;
                case date_t:
                        val = std::make_unique<DbDate>(DbDate());
                        break;
                case datelnvl_t:
                        val = std::make_unique<DbDateLnvl>(DbDateLnvl());
                        break;
                default:
                        throw std::runtime_error("invalid type");
        }
        return val;
}

bool Storage::readRow(std::fstream &fin, std::vector<Column> &cols, 
const std::vector<ColumnType> &types)
{
        ColumnHeader colHdr {};
        bool status = true;
        std::unique_ptr<DbValue> colValue;
        for (uint32_t i = 0; i < cols.size() && status; i++) {
                if (fin.read((char*)&colHdr, sizeof(colHdr))) {
                        colValue = createValue(types[i]);
                        auto buffer = std::make_unique<char[]>(colHdr.dataLen + 1);
                        if (fin.read(buffer.get(), colHdr.dataLen)) {
                                buffer.get()[colHdr.dataLen] = '\0';
                                if (!colValue->SetValue(std::string(buffer.get())))
                                        status = false;
                                else
                                        cols[i].values.push_back(std::move(colValue));
                        }
                        if (i == cols.size() - 1)
                                fin.seekg(colHdr.nextRow);
                } else {
                        ERR("fail: read col hader");
                        status = false;
                }
        }
        return status;
}

bool Storage::readTable(std::string tableName, std::vector<Column> &cols)
{
        TableHeader tblHdr {};
        bool status = true;
        fs::path tblPath = fs::path(m_dbPath) / tableName; 
        std::fstream fin(tblPath, std::ios::in | std::ios::binary);
        std::vector<ColumnType> types;
        cols.clear();
        if (fin.read((char*)&tblHdr, sizeof(tblHdr))) {
                types = readColumnTypes(fin, tblHdr.numberColumns);
                for (uint32_t i = 0; i < types.size(); i++) {
                        Column col;
                        col.name = types[i].name;
                        cols.push_back(col);
                }
                if (tblHdr.firstEntry) {
                        fin.seekg(tblHdr.firstEntry);
                        for (uint32_t i = 0; i < tblHdr.numEntries && status; i++) {
                                if (!readRow(fin, cols, types))
                                        status = false;
                        }
                }
                
        }
        return status;
}

using dbval_cmp = std::function<bool(DbValue*, DbValue*)>;
std::map<lexer::operator_t, dbval_cmp> g_cmpMap {
        { lexer::operator_t::equal, [](auto *val1, auto *val2) 
        { return *val1 == val2; }},
        { lexer::operator_t::not_equal, [](auto *val1, auto *val2) 
        { return !(*val1 == val2); }},
        { lexer::operator_t::greater, [](auto *val1, auto *val2) 
        { return !(*val1 < val2) && !(*val1 == val2); }},
        { lexer::operator_t::less, [](auto *val1, auto *val2) 
        { return *val1 < val2; }},
        { lexer::operator_t::less_equal, [](auto *val1, auto *val2) 
        { return (*val1 < val2) || (*val1 == val2); }},
        { lexer::operator_t::greater_equal, [](auto *val1, auto *val2) 
        { return (!(*val1 < val2) && !(*val1 == val2)) || (*val1 == val2); }}
};

static bool cmpColToVal(std::vector<Column> &cols, std::string colName, 
dbval_cmp fcmp, DbValue *val)
{
        bool status = false;
        for (auto &col : cols) {
                if (col.name == colName) {
                        for (uint32_t i = 0; i < col.values.size(); ) {
                                if (!fcmp(col.values[i].get(), val)) {
                                        for (auto &col : cols)
                                                col.values.erase(
                                                        col.values.begin() + i);
                                } else {
                                        i++;
                                }
                        }
                        status = true;
                        break;
                }
        }
        return status;
}

static bool cmpValToCol(std::vector<Column> &cols, DbValue *val, 
dbval_cmp fcmp, std::string colName)
{
        bool status = false;
        for (auto &col : cols) {
                if (col.name == colName) {
                        for (uint32_t i = 0; i < col.values.size(); ) {
                                if (!fcmp(val, col.values[i].get())) {
                                        for (auto &col : cols)
                                                col.values.erase(
                                                        col.values.begin() + i);
                                } else {
                                        i++;
                                }
                        }
                        status = true;
                        break;
                }
        }

        return status;
}

std::vector<Column> Storage::getCols(std::string tblName)
{
        std::vector<Column> result;
        if (!readTable(tblName, result))
                throw std::runtime_error("unable to read tables");
        return result;
}

bool Storage::dropTable(std::string tableName)
{
        bool status = false;
        fs::path tblPath = fs::path(m_dbPath) / tableName; 
        if (fs::exists(tblPath) && fs::is_regular_file(tblPath)) {
                fs::remove(tblPath);
                status = true;
        }
        return status; 
}

std::vector<Column> Storage::getByExpr(std::string tblName,
std::shared_ptr<stmt> stmt1, 
std::shared_ptr<stmt> stmt2,
std::shared_ptr<stmt> stmt3)
{
        std::string col1, col2;
        std::vector<Column> result;
        std::shared_ptr<DbValue> val1, val2;
        lexer::token *tok1, *tok2;

        lexer::token *op = dynamic_cast<lexer::token*>(stmt2.get());
        if (!op)
                throw std::runtime_error("expr: wrong operator");
        
        if ((tok1 = dynamic_cast<lexer::token*>(stmt1.get()))
        && (tok2 = dynamic_cast<lexer::token*>(stmt3.get()))) {
                if (tok1->getType() == stmt::type::identifier)
                        col1 = tok1->getValue().toString();
                else if (tok1->getType() == stmt::type::literal)
                        val1 = tok1->getValue().ToValue();
                if (tok2->getType() == stmt::type::identifier)
                        col2 = tok2->getValue().toString();
                else if (tok2->getType() == stmt::type::literal)
                        val2 = tok2->getValue().ToValue();
                if ((val1 || val2) && !(val1 && val2)) {
                        if (!readTable(tblName, result))
                                throw std::runtime_error("failed to read tbl");
                        if (val1) {
                                if (!cmpValToCol(result, val1.get(), 
                                g_cmpMap[op->getValue().toOperator()], col2))
                                        throw std::logic_error("wrong expr");
                        } else {
                                if (!cmpColToVal(result, col1, 
                                g_cmpMap[op->getValue().toOperator()], val2.get()))
                                        throw std::logic_error("wrong expr");
                        }
                } else if (val1 && val2) {
                        if (!g_cmpMap[op->getValue().toOperator()]
                        (val1.get(), val2.get())) {
                                if (!readTable(tblName, result)) {
                                        throw std::runtime_error(
                                                "failed to read tbl");
                                }
                        }
                } else {
                        throw std::runtime_error("not supported :(");
                }
        }

        return result;
}

uint32_t Storage::writeTable(std::fstream &fio, std::vector<Column> &cols)
{
        uint32_t rowsCount = cols.at(0).values.size();
        for (uint32_t i = 0; i < rowsCount; i++) {
                std::vector<uint32_t> hdrPos;
                for (auto &col : cols) {
                        hdrPos.push_back(fio.tellg());
                        std::string val = col.values[i]->GetValue();
                        ColumnHeader cHdr { (uint32_t)val.size(), 0 };
                        fio.write((char*)&cHdr, sizeof(cHdr));
                        fio.write(val.c_str(), val.size());
                }
                uint32_t endPos = (i + 1 == rowsCount) 
                ? std::streampos(0) : fio.tellg();
                uint32_t oldPos = fio.tellg();
                for (auto pos : hdrPos) {
                        fio.seekg(pos + sizeof(uint32_t));
                        fio.write((char*)&endPos, sizeof(endPos));
                }
                fio.seekg(oldPos);
        }
        return rowsCount;
}

bool Storage::writeValues(std::vector<Column> &cols, std::string tableName)
{
        bool status = true;

        fs::path tblPath = fs::path(m_dbPath) / tableName; 
        std::fstream fio(tblPath, std::ios::in 
        | std::ios::out | std::ios::binary);

        TableHeader tblHdr {};
        if (fio.read((char*)&tblHdr, sizeof(tblHdr))) {
                fio.seekg(tblHdr.firstEntry);
                writeTable(fio, cols);
                fio.seekg(0);
                tblHdr.numEntries = cols.at(0).values.size();
                if (!fio.write((char*)&tblHdr, sizeof(tblHdr)))
                        status = false;
        }

        return status;
}

bool Storage::insertValues(std::vector<Column> &cols, std::string tableName)
{
        bool status = false;

        std::vector<Column> rows;
        if (!readTable(tableName, rows))
                return status;

        fs::path tblPath = fs::path(m_dbPath) / tableName; 
        std::fstream fio(tblPath, std::ios::in 
        | std::ios::out | std::ios::binary);

        TableHeader tblHdr {};
        if (fio.read((char*)&tblHdr, sizeof(tblHdr))) {

                if (cols.size() != tblHdr.numberColumns)
                        throw std::logic_error("invalud number of values");

                auto colTypes = readColumnTypes(fio, tblHdr.numberColumns);
                uint32_t freePos = fio.tellg();

                for (uint32_t i = 0; i < colTypes.size(); i++) {
                        auto colValue = 
                        createValue(colTypes.at(i));
                        std::string name = colTypes.at(i).name;
                        auto itCol = std::find_if(cols.begin(), cols.end(),
                        [&](auto &col) { return col.name == name; } );

                        if (itCol != cols.end() &&  !itCol->values.empty())
                                colValue->SetValue(itCol->values.at(0)->GetValue());

                        if (i == 0) {
                                auto &&rVals = rows.at(0).values;
                                auto itVal = std::find_if(rVals.begin(), rVals.end(),
                                [&](auto &val) 
                                { return *val.get() == colValue.get(); });
                                if (itVal != rVals.end()) {
                                        ERR("prim key already exist");
                                        return false;
                                }
                        }
                        rows.at(i).values.push_back(std::move(colValue));
                }
                if (writeTable(fio, rows) != 0)
                        status = true;

                tblHdr.firstEntry = freePos;
                tblHdr.numEntries += 1;
                fio.seekg(0);
                if (!fio.write((char*)&tblHdr, sizeof(tblHdr)))
                        status = false;
        }

        return status;
}


bool Storage::createTable(stmt *identifier, std::vector<ColumnType> cols)
{
        lexer::token *nameTok;
        bool status = false;

        if ((nameTok = dynamic_cast<lexer::token*>(identifier))) {
                std::string tableName = nameTok->getValue().toString(); 
                fs::path tblPath = fs::path(m_dbPath) / tableName; 

                if (tableName.empty() || cols.empty()) {
                        ERR("create invalid params");
                        return status;
                }
                
                if (fs::exists(tblPath))
                        throw std::runtime_error("already exist");

                if (!tableName.empty() && !fs::exists(tblPath) && !cols.empty()) {
                        std::ofstream fout(tblPath, std::ios::out | std::ios::binary);
                        TableHeader hdr { 0, 0, cols.size() };
                        if (fout.write((char*)&hdr, sizeof(hdr))) {
                                status = true;
                                for (auto &col : cols) {
                                        TypeHeader thdr { col.type, 
                                        col.name.size() + 1 };
                                        if (!fout.write((char*)&thdr, sizeof(thdr))) {
                                                status = false;
                                                break;
                                        }
                                        if (!fout.write((char*)col.name.c_str(), 
                                        col.name.size() + 1)) {
                                                status = false;
                                                break;
                                        }
                                }
                        }
                } else {
                        ERR("wrong create query");
                }
        }
        
        return status;
}

bool stmtToColumnDef(stmt *identifier, stmt *type, Storage::ColumnType &colDef)
{
        bool status = false;
        lexer::token *tokName, *tokType;
        if ((tokName = dynamic_cast<lexer::token*>(identifier)),
        (tokType = dynamic_cast<lexer::token*>(type))) {
                colDef.name = tokName->getValue().toString();
                switch (tokType->getValue().toKeyword()) {
                case lexer::keyword_t::string:
                        colDef.type = Storage::string_t;
                        status = true;
                        break;
                case lexer::keyword_t::integer:
                        colDef.type = Storage::integer_t;
                        status = true;
                        break;
                case lexer::keyword_t::real:
                        colDef.type = Storage::real_t;
                        status = true;
                        break;
                case lexer::keyword_t::chr:
                        colDef.type = Storage::char_t;
                        status = true;
                        break;
                case lexer::keyword_t::date:
                        colDef.type = Storage::date_t;
                        status = true;
                        break;
                case lexer::keyword_t::datelnvl:
                        colDef.type = Storage::datelnvl_t;
                        status = true;
                        break;
                }
        }
        return status;
}
