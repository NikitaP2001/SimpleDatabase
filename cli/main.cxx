#include <vector>
#include <filesystem>
#include <fstream>

#include "lexer.h"
#include "database.h"
#include "parser.h"
#include "error.hpp"

using namespace lexer;

namespace fs = std::filesystem;

constexpr static const char *g_kDbName = "testdb";

static bool tokenize(IN std::istream *sin, OUT std::vector<lexer::token> &tokens)
{
        Lexer lex;
        std::string squery;
        bool isEnd = false;

        if (std::getline(*sin, squery)) {
                lexer::token tok(stmt::type::type_error, token::value(""));
                auto sit = squery.begin();
                while (sit != squery.end()) {
                        sit = lex.getToken(sit, squery.end(), tok);
                        if (sit == squery.end()) {
                                std::cout << "syntax error" << std::endl;
                                break;
                        }
                        sit++;
                        tokens.push_back(tok);
                }
                isEnd = true;
        }
        return isEnd; 
}

static void printResponse(database::Response &res)
{
        Column col;
        while (res.getColumn(col)) {
                std::cout << "column: " << col.name 
                << std::endl;
                for (auto val : col.values) {
                        std::cout << val->GetValue() << " ";
                }
                std::cout << std::endl;
        }
}

static bool execute(Storage &st, std::vector<lexer::token> &tokens) {
        bool result = false;
        parser::Parser prs(std::make_shared<Storage>(st));

        if (tokens.begin() != tokens.end()) {
                auto itTokBeg = tokens.begin();
                auto root = prs.getTree(stmt::type::query, itTokBeg, tokens.end());
                if (root && itTokBeg == tokens.end()) {
                        try {
                                auto response = root->execute("");
                                printResponse(response);
                                result = true; 
                        } catch (std::exception &ex) {
                                std::cout << "[-] " << ex.what() << std::endl;
                        }
                        
                } else {
                        std::cout << "parsing error" << std::endl;
                }
        }
        return result;
}

static bool processInput(std::istream *sin) {
        bool result = false;
        Storage st(g_kDbName);
        std::vector<lexer::token> tokens;

        while (tokenize(sin, tokens)) {
                result = execute(st, tokens);
                if (!result)
                        break;
                tokens.clear();
        }

        return result;
}

static std::string processArgs(int argc, char *argv[])
{
        std::string fName;
        for (int i = 0; i < argc; i++) {
                if (std::string(argv[i]) == "-f" && i + 1 < argc)
                        fName = argv[i + 1];
        }
        return fName;
}

int main(int argc, char *argv[])
{
        std::string queryFile;
        std::ifstream fin;
        std::istream *sin = nullptr;

        queryFile = processArgs(argc, argv);
        if (!queryFile.empty()) {
                if (fs::exists(queryFile)) {
                        fin = std::ifstream(queryFile, std::ios::in);
                        sin = &fin;
                } else {
                        std::cerr << "Can not reach file " << queryFile << std::endl;
                        exit(1);
                }
        } else
                sin = &std::cin;

        std::cout << (processInput(sin) ? "SUCCESS" : "FAILTURE");
        exit(0);
}