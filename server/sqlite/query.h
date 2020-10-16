#ifndef SQLITEQUERY_H
#define SQLITEQUERY_H

#include <cstdint>
#include <string>
#include <optional>

typedef struct sqlite3_stmt sqlite3_stmt;

namespace ThorQ {
namespace SQLite {

class Connection;
class Value;

class Query
{
    friend Connection;
    Query() = default;
    Query(const Query& other) = default;
    Query& operator=(const Query& other) = default;
public:
    Query(Connection* connection, const char* statement);

    bool isValid();

    void bind(int index, const Value& value);
    void bind(int index, const char* value);

    void step();

    Value getColumn(int col);
private:
    Connection* m_connection;
    sqlite3_stmt* m_stmt;
};
}
}

#endif // SQLITEQUERY_H
