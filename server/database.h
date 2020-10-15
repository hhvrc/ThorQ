#ifndef DATABASE_H
#define DATABASE_H

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
typedef struct sqlite3_value sqlite3_value;

namespace ThorQ {
namespace SQLite {
bool Initialize(const char* path) noexcept;
class Connection
{
public:
    Connection(const char* path);
    ~Connection();

    enum class OpenMode
    {
        READONLY,
        READWRITE,
        CREATE,
    };

    bool open(OpenMode openMode = OpenMode::READONLY);

    bool ready();

    bool ExecuteNonQuery(const char* statement);

private:
    const char* m_path;

    sqlite3* m_db;
};
class Query
{
private:
    sqlite3_stmt* m_stmt:
};
class Value
{
private:
    sqlite3_value* m_value;
};
}
}

#endif // DATABASE_H
