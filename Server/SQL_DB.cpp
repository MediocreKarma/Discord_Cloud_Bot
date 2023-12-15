#include "SQL_DB.hpp"

SQL_DB::SQL_DB(const std::string& _filename) : 
    filename(_filename), db(nullptr), stmt(nullptr), is_open(false), nextRow_rc(0), dbMutex(), isLocked(false) {
    int rc = sqlite3_open(filename.c_str(), &db);
    is_open = (rc == SQLITE_OK);
}

SQL_DB::~SQL_DB() {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    db = nullptr;
    stmt = nullptr;
}

SQL_DB::SQL_DB(SQL_DB&& other) : 
    filename(), db(other.db), stmt(other.stmt), nextRow_rc(other.nextRow_rc), dbMutex(), isLocked(other.isLocked) {
    other.lock();
    filename = std::move(other.filename);
    other.db = nullptr;
    other.stmt = nullptr;
    other.isLocked = true;
}

SQL_DB& SQL_DB::operator = (SQL_DB&& other)  {
    // lock in constructor until the other one is done
    // and leave the other one locked forever
    db = other.db;
    stmt = other.stmt;
    nextRow_rc = other.nextRow_rc;
    isLocked = other.isLocked;
    other.lock();
    other.db = nullptr;
    other.stmt = nullptr;
    other.isLocked = true;
    return *this;
}

bool SQL_DB::createStatement(const std::string& stmtStr) {
    if (!isLocked) {
        throw std::logic_error("Not allowed to do SQL requests without first locking");
    }
    if (stmt != nullptr) {
        sqlite3_finalize(stmt);
        stmt = nullptr;
    }
    sqlite3_stmt* ptr = nullptr;
    int rc = sqlite3_prepare_v2(db, stmtStr.c_str(), stmtStr.size(), &ptr, nullptr);
    stmt = ptr;
    ptr = nullptr;
    nextRow_rc = 0;
    return (rc == SQLITE_OK);
}

bool SQL_DB::nextRow() {
    if (!isLocked) {
        throw std::logic_error("Not allowed to do SQL requests without first locking");
    }
    if (nextRow_rc == SQLITE_DONE) {
        return false;
    }
    nextRow_rc = sqlite3_step(stmt);
    while (nextRow_rc == SQLITE_BUSY) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        nextRow_rc = sqlite3_step(stmt);
    }
    return nextRow_rc == SQLITE_ROW || nextRow_rc == SQLITE_DONE;
}

bool SQL_DB::isOpen() const {
    return is_open;
}

#include <iostream>

void SQL_DB::lock() {
    dbMutex.lock();
    isLocked = true;
}

void SQL_DB::unlock() {
    dbMutex.unlock();
    isLocked = false;
}

std::string SQL_DB::name() const {
    return filename;
}

template<>
const void* SQL_DB::extract<const void*>(int index) {
    if (!isLocked) {
        throw std::logic_error("Not allowed to do SQL requests without first locking");
    }
    const void* blob = sqlite3_column_blob(stmt, index);
    return blob;
}

template<>
std::string SQL_DB::extract<std::string>(int index) {
    if (!isLocked) {
        throw std::logic_error("Not allowed to do SQL requests without first locking");
    }
    const char* data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
    if (data == nullptr) {
        return "";
    }
    return data;
}

template<>
int64_t SQL_DB::extract<int64_t>(int index) {
    if (!isLocked) {
        throw std::logic_error("Not allowed to do SQL requests without first locking");
    }
    return sqlite3_column_int64(stmt, index);
}