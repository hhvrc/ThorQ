#ifndef SQLITETRANSACTION_H
#define SQLITETRANSACTION_H

namespace ThorQ {
namespace SQLite {

class Connection;
class Value;

class Transaction
{
    friend Connection;
    Transaction() = default;
    Transaction(const Transaction& other) = default;
    Transaction& operator=(const Transaction& other) = default;
public:
    Transaction(Connection& connection);
    ~Transaction();

    bool isOpen() const;

    bool commit();
    bool rollback();
private:
    Connection* m_connection;
};
}
}

#endif // SQLITETRANSACTION_H
