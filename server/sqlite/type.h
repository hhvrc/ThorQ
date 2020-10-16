#ifndef SQLITETYPE_H
#define SQLITETYPE_H

namespace ThorQ {
namespace SQLite {
enum class Type : int
{
    INVALID = 0,
    INTEGER = 1,
    FLOAT   = 2,
    TEXT    = 3,
    BLOB    = 4,
    NULL    = 5
};
}
}

#endif // SQLITETYPE_H
