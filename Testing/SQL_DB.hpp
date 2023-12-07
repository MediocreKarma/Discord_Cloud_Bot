#ifndef _SQL_DB__
#define _SQL_DB__

#include <sqlite3.h>
#include <string>
#include <string.h>
#include <thread>

//////////////////////////////////////
/// wrapper class for sql_db 
//////////////////////////////////////
class SQL_DB {
public:
    SQL_DB() = delete;
    SQL_DB(const std::string& filename);
    ~SQL_DB();

    SQL_DB(const SQL_DB&) = delete;
    SQL_DB& operator = (const SQL_DB&) = delete;

    SQL_DB(SQL_DB&&);
    SQL_DB& operator = (SQL_DB&&);

    bool createStatement(const std::string& sql_statement);
    bool nextRow();

    template<typename T>
    T extract(int index);

private:
    sqlite3* db;
    sqlite3_stmt* stmt;
    bool is_open;
    int nextRow_rc;
};

















#endif