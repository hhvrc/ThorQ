#ifndef SQLITECONNECTION_H
#define SQLITECONNECTION_H

#include <cstdint>
#include <string>

typedef struct sqlite3 sqlite3;

namespace ThorQ {
namespace SQLite {
class Transaction;
class Query;
class Value;

class Connection
{
    friend Transaction;
    friend Query;

public:
    enum OpenMode : int
    {
        READONLY  = 1,
        READWRITE = 2,
        CREATE    = 4,
    };

    Connection(const char* apFilename, int aFlags = OpenMode::READONLY, int aBusyTimeoutMs = 0);
    ~Connection();

    bool isOpen() const;

    bool setBusyTimeout(const int aBusyTimeoutMs);

    Transaction transaction();

    Query query(const char* statement);
    bool execute(const char* statement);
    bool tableExists(const char* apTableName);

    std::int64_t lastInsertedRowId() const;

    const char* lastError() const;
private:
    sqlite3* m_db;
    Transaction* m_transaction;
};
}
}

#endif // SQLITECONNECTION_H
