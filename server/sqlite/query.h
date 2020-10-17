#ifndef SQLITEQUERY_H
#define SQLITEQUERY_H

#include <cstdint>
#include <string>
#include <vector>

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

    bool bind(int index, std::int32_t value);
    bool bind(int index, std::int64_t value);
    bool bind(int index, double value);
    bool bind(int index, const std::string& value);
    bool bind(int index, const std::vector<std::uint8_t>& value);
    bool bind(int index, const Value& value);

    bool step();

    int columnCount() const;
    SQLite::Type getType(int col);
    SQLite::Column column(int col);
private:
    sqlite3_stmt* m_stmt;
    int m_ncols;
};
}
}

#endif // SQLITEQUERY_H
