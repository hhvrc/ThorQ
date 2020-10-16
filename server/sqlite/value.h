#ifndef SQLITEVALUE_H
#define SQLITEVALUE_H

#include <cstdint>
#include <string>
#include <vector>

#include "type.h"

typedef struct sqlite3_value sqlite3_value;

namespace ThorQ {
namespace SQLite {
class Query;
class Column;

class Value
{
    friend SQLite::Query;
    friend SQLite::Column;

    Value(const sqlite3_value* value);
public:
    Value(); ///< Constructs a NULL value
    Value(std::int32_t val);
    Value(std::int64_t val);
    Value(double val);
    Value(const std::string& text);
    Value(const std::vector<std::uint8_t>& blob);
    Value(const Value& other);

    void null();
    Value& operator=(std::int32_t val);
    Value& operator=(std::int64_t val);
    Value& operator=(double val);
    Value& operator=(const std::string& text);
    Value& operator=(const std::vector<std::uint8_t>& blob);
    Value& operator=(const Value& other);

    bool isValid() const;

    SQLite::Type type() const;

    int getInt();
    const char* getString();
private:
    sqlite3_value* m_value;
};
}
}

#endif // SQLITEVALUE_H
