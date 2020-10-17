#ifndef SQLITETYPE_H
#define SQLITETYPE_H

namespace ThorQ {
namespace SQLite {
enum class Type : int
{
    Invalid  = 0,
    Integer = 1,
    Float    = 2,
    Text     = 3,
    Blob     = 4,
    Null     = 5
};
}
}

#endif // SQLITETYPE_H
