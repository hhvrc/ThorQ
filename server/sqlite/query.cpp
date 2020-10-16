#include "query.h"

#include <cstring>

#include "internal/sqlite3.h"

#include "connection.h"

ThorQ::SQLite::Query::Query(ThorQ::SQLite::Connection* connection, const char *statement)
    : m_connection(connection)
    , m_stmt(nullptr)
{
    if (connection != nullptr)
    {
        const char* tail;
        std::size_t nByte = strlen(statement);

        if (sqlite3_prepare_v3(m_connection->m_db, statement, nByte, 0, &m_stmt, &tail) != SQLITE_OK)
        {
            sqlite3_finalize(m_stmt);
            m_connection = nullptr;
            m_stmt = nullptr;
        }
    }
}

bool ThorQ::SQLite::Query::valid()
{
    return m_connection != nullptr && m_stmt != nullptr;
}
