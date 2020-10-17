#ifndef SQLITECOLUMN_H
#define SQLITECOLUMN_H

#include <cstdint>
#include <string>
#include <vector>

#include "type.h"

typedef struct sqlite3_stmt sqlite3_stmt;

namespace ThorQ {
namespace SQLite {
class Query;
class Value;

class Column
{
    friend SQLite::Query;
    friend SQLite::Value;
public:
    Column(SQLite::Query* query, int col);
    Column(const Column& other) = default;
    Column& operator=(const Column& other) = default;
    bool isValid() const;

    SQLite::Type type() const;

    std::int32_t getInt();
    std::int64_t getInt64();
    double       getDouble();
    std::string  getText();
    std::vector<std::uint8_t> getBlob();

    SQLite::Value getValue(); ///< Allocates a copy of the value at the row, making it independant from the Query
private:
    SQLite::Query* m_query;
    int m_col;
};
}
}

#endif // SQLITECOLUMN_H
