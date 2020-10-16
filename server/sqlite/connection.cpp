#include "connection.h"

#include "internal/sqlite3.h"

#include "transaction.h"
#include "query.h"
#include "value.h"

ThorQ::SQLite::Connection::Connection(const char* apFilename,
                                      int aFlags,             /* = OpenMode::READONLY */
                                      int aBusyTimeoutMs,     /* = 0                  */
                                      const char* apVfs       /* = nullptr            */)
    : m_db(nullptr)
    , m_transaction(nullptr)
{
    if (apFilename != nullptr)
    {
        if (sqlite3_open_v2(apFilename, &m_db, aFlags, apVfs) != SQLITE_OK)
        {
            sqlite3_close_v2(m_db);
            m_db = nullptr;
            return;
        }

        if (aBusyTimeoutMs > 0)
        {
            setBusyTimeout(aBusyTimeoutMs);
        }
    }
}

ThorQ::SQLite::Connection::~Connection()
{
    if (m_transaction != nullptr)
    {
        m_transaction->rollback();
    }
    if (m_db != nullptr)
    {
        sqlite3_close_v2(m_db);
    }
}

bool ThorQ::SQLite::Connection::isOpen() const
{
    return m_db != nullptr;
}

bool ThorQ::SQLite::Connection::setBusyTimeout(int aBusyTimeoutMs)
{
    if (m_db != nullptr)
    {
        return sqlite3_busy_timeout(m_db, aBusyTimeoutMs);
    }

    return false;
}

ThorQ::SQLite::Transaction ThorQ::SQLite::Connection::transaction()
{
    return Transaction(*this);
}

ThorQ::SQLite::Query ThorQ::SQLite::Connection::query(const char* statement)
{
    return Query(statement, *this);
}

bool ThorQ::SQLite::Connection::execute(const char* statement)
{
    return sqlite3_exec(m_db, statement, nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool ThorQ::SQLite::Connection::tableExists(const char* apTableName)
{
    Query query("SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?", *this);
    query.bind(1, apTableName);
    (void)query.step(); // Cannot return false, as the above query always return a result
    return (1 == query.getColumn(0).getInt());
}

uint64_t ThorQ::SQLite::Connection::lastInsertedRowId() const
{
    if (m_db != nullptr)
    {
        return sqlite3_last_insert_rowid(m_db);
    }

    return 0;
}

const char *ThorQ::SQLite::Connection::lastError() const
{
    if (m_db != nullptr)
    {
        return sqlite3_errmsg(m_db);
    }

    return "Database is not open!";
}
