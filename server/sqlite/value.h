#ifndef SQLITEVALUE_H
#define SQLITEVALUE_H

#include <cstdint>
#include <string>
#include <optional>

typedef struct sqlite3_value sqlite3_value;

namespace ThorQ {
namespace SQLite {
class Query;

class Value
{
    friend Query;

    Value() = default;
    Value(const Value& other) = default;
    Value& operator=(const Value& other) = default;
public:
    enum Type
    {

    };
    Type type();

    int getInt();
    const char* getString();
private:
    sqlite3_value* m_value;
};
}
}

#endif // SQLITEVALUE_H
