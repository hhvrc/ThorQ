#ifndef SQLITEQUERY_H
#define SQLITEQUERY_H

#include <cstdint>
#include <string>

#include "type.h"

typedef struct sqlite3_stmt sqlite3_stmt;

namespace ThorQ {
namespace SQLite {

class Connection;
class Column;
class Value;

class Query
{
    friend Connection;
    friend Column;

    Query();
    Query(const Query& other) = default;
    Query& operator=(const SQLite::Query& other) = default;
public:
    Query(const char* statement, SQLite::Connection& connection);

    bool isValid() const;

    bool bind(int index, const Value& value);
    bool bind(int index, const char* value);

    bool step();

    int columnCount() const;
    SQLite::Column getColumn(int col);
private:
    sqlite3_stmt* m_stmt;
    int m_ncols;
};
}
}

#endif // SQLITEQUERY_H
