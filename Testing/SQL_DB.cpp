#include "SQL_DB.hpp"

SQL_DB::SQL_DB(const std::string& filename) : db(nullptr), stmt(nullptr), is_open(false), nextRow_rc(0) {
    int rc = sqlite3_open(filename.c_str(), &db);
    is_open = (rc == SQLITE_OK);
    if (!is_open) {
        return;
    }
}

SQL_DB::~SQL_DB() {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    db = nullptr;
    stmt = nullptr;
}

SQL_DB::SQL_DB(SQL_DB&& other) {
    *this = std::move(other);
}

SQL_DB& SQL_DB::operator = (SQL_DB&& other) {
    memcpy(this, &other, sizeof(SQL_DB));
    memset(&other, 0, sizeof(SQL_DB));
}

bool SQL_DB::createStatement(const std::string& stmtStr) {
    if (stmt != nullptr) {
        sqlite3_finalize(stmt);
        stmt = nullptr;
    }
    int rc = sqlite3_prepare_v2(db, stmtStr.c_str(), stmtStr.size(), &stmt, nullptr);
    nextRow_rc = 0;
    return (rc == SQLITE_OK);
}

bool SQL_DB::nextRow() {
    if (nextRow_rc == SQLITE_DONE) {
        return false;
    }
    nextRow_rc = sqlite3_step(stmt);
    while (nextRow_rc == SQLITE_BUSY) {
        nextRow_rc = sqlite3_step(stmt);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return nextRow_rc == SQLITE_ROW;
}

template<>
const void* SQL_DB::extract<const void*>(int index) {
    const void* blob = sqlite3_column_blob(stmt, index);
    if (blob == nullptr) {
        return nullptr;
    }
    return *(T*)blob;
}

template<>
std::string SQL_DB::extract(int index) {
    const char* data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
    if (data == nullptr) {
        return "";
    }
    return data;
}

template<>
int64_t SQL_DB::extract(int index) {
    return sqlite3_column_int64(stmt, index);
}