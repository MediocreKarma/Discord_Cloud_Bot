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

    bool isOpen() const;

    // unorthodox methods, but I want the SQL_STATEMENT to survive
    // even if there may be multiple rows to parse through
    void lock();
    void unlock();

    std::string name() const;


    /// @brief extract column value
    /// @tparam T type of data to be extracted
    /// T may be a std::string, a int64_t, or a const void*
    /// @param index Column index in the database
    /// @return T
    template<typename T>
    T extract(int index);

private:
    std::string filename;
    sqlite3* db;
    sqlite3_stmt* stmt;
    bool is_open;
    int nextRow_rc;
    std::mutex dbMutex;
    bool isLocked;
};

template<>
const void* SQL_DB::extract<const void*>(int index);

template<>
int64_t SQL_DB::extract<int64_t>(int index);

template<>
std::string SQL_DB::extract<std::string>(int index);

#endif