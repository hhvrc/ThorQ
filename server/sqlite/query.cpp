#include "query.h"

#include <cstring>

#include "internal/sqlite3.h"

#include "connection.h"
#include "column.h"
#include "value.h"

ThorQ::SQLite::Query::Query()
    : m_stmt(nullptr)
    , m_ncols(0)
{

}

ThorQ::SQLite::Query::Query(const char *statement, ThorQ::SQLite::Connection& connection)
    : m_stmt(nullptr)
    , m_ncols(0)
{
    const char* tail;
    std::size_t nByte = strlen(statement);

    if (sqlite3_prepare_v3(connection.m_db, statement, nByte, 0, &m_stmt, &tail) != SQLITE_OK)
    {
        sqlite3_finalize(m_stmt);
        m_stmt = nullptr;
    }
}

bool ThorQ::SQLite::Query::isValid() const
{
    return m_stmt != nullptr;
}

bool ThorQ::SQLite::Query::bind(int index, int32_t value)
{
    if (isValid())
    {
        return sqlite3_bind_int(m_stmt, index, value) == SQLITE_OK;
    }
    return false;
}

bool ThorQ::SQLite::Query::bind(int index, int64_t value)
{
    if (isValid())
    {
        return sqlite3_bind_int64(m_stmt, index, value) == SQLITE_OK;
    }
    return false;
}

bool ThorQ::SQLite::Query::bind(int index, double value)
{
    if (isValid())
    {
        return sqlite3_bind_double(m_stmt, index, value) == SQLITE_OK;
    }
    return false;
}

bool ThorQ::SQLite::Query::bind(int index, const std::string& value)
{
    if (isValid())
    {
        return sqlite3_bind_text(m_stmt, index, value.data(), value.size(), nullptr) == SQLITE_OK;
    }
    return false;
}

bool ThorQ::SQLite::Query::bind(int index, const std::vector<uint8_t> &value)
{
    if (isValid())
    {
        return sqlite3_bind_blob(m_stmt, index, value.data(), value.size(), nullptr) == SQLITE_OK;
    }
    return false;
}

bool ThorQ::SQLite::Query::bind(int index, const ThorQ::SQLite::Value &value)
{
    if (isValid())
    {
        return sqlite3_bind_value(m_stmt, index, value.m_value) == SQLITE_OK;
    }
    return false;
}

bool ThorQ::SQLite::Query::step()
{
    if (isValid() && sqlite3_step(m_stmt) == SQLITE_OK)
    {
        m_ncols = sqlite3_column_count(m_stmt);
        return true;
    }
    m_ncols = 0;
    return false;
}

int ThorQ::SQLite::Query::columnCount() const
{
    return m_ncols;
}

ThorQ::SQLite::Type ThorQ::SQLite::Query::getType(int col)
{
    if (isValid())
    {
        return (SQLite::Type)sqlite3_column_type(m_stmt, col);
    }
    return SQLite::Type::Invalid;
}

ThorQ::SQLite::Column ThorQ::SQLite::Query::column(int col)
{
    return ThorQ::SQLite::Column(this, col);
}
