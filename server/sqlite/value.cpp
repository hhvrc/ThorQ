#include "value.h"

#include <exception>

#include "internal/sqlite3.h"

ThorQ::SQLite::Value::Value(const sqlite3_value* value)
    : m_value(nullptr)
{
    if (value != nullptr)
    {
        m_value = sqlite3_value_dup(value);
    }
}

ThorQ::SQLite::Value::Value()
{

}

ThorQ::SQLite::Value::Value(int32_t val)
{

}

ThorQ::SQLite::Value::Value(int64_t val)
{

}

ThorQ::SQLite::Value::Value(double val)
{

}

ThorQ::SQLite::Value::Value(const ThorQ::SQLite::Value& other)
    : m_value(nullptr)
{
    if (other.isValid())
    {
        m_value = sqlite3_value_dup(other.m_value);
    }
}

ThorQ::SQLite::Value& ThorQ::SQLite::Value::operator=(const ThorQ::SQLite::Value& other)
{
    if (isValid())
    {
        sqlite3_value_free(m_value);
        m_value = nullptr;
    }

    if (other.isValid())
    {
        m_value = sqlite3_value_dup(other.m_value);
    }

    return *this;
}

bool ThorQ::SQLite::Value::isValid() const
{
    return m_value != nullptr;
}

ThorQ::SQLite::Type ThorQ::SQLite::Value::type() const
{
    if (isValid()) return (SQLite::Type)sqlite3_value_type(m_value);
    return SQLite::Type::INVALID;
}

