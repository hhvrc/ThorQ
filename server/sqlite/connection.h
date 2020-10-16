#ifndef SQLITECONNECTION_H
#define SQLITECONNECTION_H

#include <cstdint>
#include <string>
#include <optional>

typedef struct sqlite3 sqlite3;

namespace ThorQ {
namespace SQLite {
class Query;
class Value;

class Connection
{
    friend Query;

    Connection(sqlite3* connection);
public:
    enum OpenMode : int
    {
        READONLY,
        READWRITE,
        CREATE,
    };

    static std::optional<Connection> Open(const char* apFilename, const int aFlags = OpenMode::READONLY, const int aBusyTimeoutMs = 0, const char* apVfs = nullptr);
    ~Connection();

    bool setBusyTimeout(const int aBusyTimeoutMs);

    Query query(const char* statement);
    bool execute(const char* statement);
    bool tableExists(const char* apTableName);

    std::uint64_t lastInsertedRowId();
private:
    sqlite3* m_db;
};
}
}

#endif // SQLITECONNECTION_H
