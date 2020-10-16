#include "connection.h"

#include "internal/sqlite3.h"

#include "query.h"
#include "value.h"

ThorQ::SQLite::Connection::Connection(sqlite3* connection)
    : m_db(connection)
{
}

std::optional<ThorQ::SQLite::Connection> ThorQ::SQLite::Connection::Open(const char* apFilename,
                                                            int aFlags,             /* = OpenMode::READONLY */
                                                            int aBusyTimeoutMs,     /* = 0                  */
                                                            const char* apVfs       /* = nullptr            */)
{
    if (apFilename == nullptr)
    {
        return {};
    }

    sqlite3* db;
    if (sqlite3_open_v2(apFilename, &db, aFlags, apVfs) != SQLITE_OK)
    {
        sqlite3_close_v2(db);
        return {};
    }

    ThorQ::SQLite::Connection connection(db);

    if (aBusyTimeoutMs > 0)
    {
        connection.setBusyTimeout(aBusyTimeoutMs);
    }

    return connection;
}

ThorQ::SQLite::Connection::~Connection()
{
    if (m_db != nullptr)
    {
        sqlite3_close_v2(m_db);
    }
}

bool ThorQ::SQLite::Connection::setBusyTimeout(int aBusyTimeoutMs)
{
    if (m_db != nullptr)
    {
        return sqlite3_busy_timeout(m_db, aBusyTimeoutMs);
    }

    return false;
}

ThorQ::SQLite::Query ThorQ::SQLite::Connection::query(const char* statement)
{
    return Query(this, statement);
}

bool ThorQ::SQLite::Connection::execute(const char* statement)
{
    return sqlite3_exec(m_db, statement, nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool ThorQ::SQLite::Connection::tableExists(const char* apTableName)
{
    Query query(this, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?");
    query.bind(1, apTableName);
    (void)query.step(); // Cannot return false, as the above query always return a result
    return (1 == query.getColumn(0).getInt());
}

uint64_t ThorQ::SQLite::Connection::lastInsertedRowId()
{
    if (m_db != nullptr)
    {
        return sqlite3_last_insert_rowid(m_db);
    }

    return 0;
}
