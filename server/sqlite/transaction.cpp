#include "transaction.h"

#include "connection.h"

ThorQ::SQLite::Transaction::Transaction(ThorQ::SQLite::Connection& connection)
    : m_connection(nullptr)
{
    if (connection.execute("BEGIN TRANSACTION;"))
    {
        m_connection = &connection;
        m_connection->m_transaction = this;
    }
}

ThorQ::SQLite::Transaction::~Transaction()
{
    rollback();
}

bool ThorQ::SQLite::Transaction::isOpen() const
{
    return m_connection != nullptr;
}

bool ThorQ::SQLite::Transaction::commit()
{
    bool retval = false;

    if (isOpen())
    {
        SQLite::Connection* connection = m_connection;
        m_connection = nullptr;
        retval = connection->execute("COMMIT;");
        connection->m_transaction = nullptr;
    }

    return retval;
}

bool ThorQ::SQLite::Transaction::rollback()
{
    bool retval = false;

    if (isOpen())
    {
        SQLite::Connection* connection = m_connection;
        m_connection = nullptr;
        retval = connection->execute("ROLLBACK;");
        connection->m_transaction = nullptr;
    }

    return retval;
}
