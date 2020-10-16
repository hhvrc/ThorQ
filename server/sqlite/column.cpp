#include "column.h"

#include "internal/sqlite3.h"

#include "query.h"
#include "value.h"

ThorQ::SQLite::Column::Column(ThorQ::SQLite::Query* query, int col)
    : m_query(query)
    , m_col(col)
{
}

bool ThorQ::SQLite::Column::isValid() const
{
    return m_query != nullptr
        && m_query->m_stmt != nullptr
        && m_col <= m_query->m_ncols;
}

ThorQ::SQLite::Type ThorQ::SQLite::Column::type() const
{
    if (isValid())
    {
        return (SQLite::Type)sqlite3_column_type(m_query->m_stmt, m_col);
    }
    return SQLite::Type::INVALID;
}

std::int32_t ThorQ::SQLite::Column::getInt()
{
    return sqlite3_column_int(m_query->m_stmt, m_col);
}

int64_t ThorQ::SQLite::Column::getInt64()
{
    return sqlite3_column_int64(m_query->m_stmt, m_col);
}

double ThorQ::SQLite::Column::getDouble()
{
    return sqlite3_column_double(m_query->m_stmt, m_col);
}

void ThorQ::SQLite::Column::getText(std::string& textOut)
{
    textOut.resize(sqlite3_column_bytes(m_query->m_stmt, m_col));

    memcpy(textOut.data(), sqlite3_column_text(m_query->m_stmt, m_col), textOut.size());
}

void ThorQ::SQLite::Column::getBlob(std::vector<std::uint8_t>& blobOut)
{
    blobOut.resize(sqlite3_column_bytes(m_query->m_stmt, m_col));

    memcpy(blobOut.data(), sqlite3_column_blob(m_query->m_stmt, m_col), blobOut.size());
}

ThorQ::SQLite::Value ThorQ::SQLite::Column::getValue()
{
    return SQLite::Value(sqlite3_column_value(m_query->m_stmt, m_col));
}
