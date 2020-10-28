#include "relationship.h"

#include "sqlite/connection.h"
#include "sqlite/column.h"
#include "sqlite/query.h"

#include "account.h"

std::shared_ptr<ThorQ::Relationship> ThorQ::Relationship::NewRelationship(std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target)
{
    ThorQ::SQLite::Connection connection("database.db", ThorQ::SQLite::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return nullptr;
    }

    SQLite::Query ensure = connection.query("INSERT OR IGNORE INTO relationships(uuid, source, target) VALUES (?1, ?3, ?4),(?2, ?4, ?3);");
    ensure.bind(1, "please generate some text");
    ensure.bind(2, "and some more generated text");
    ensure.bind(3, source->databaseId());
    ensure.bind(4, target->databaseId());
    ensure.step();
    ensure.finalize();

    SQLite::Query fetch = connection.query("SELECT db_id, uuid, source, target, status, authority FROM relationships WHERE uuid LIKE ?1 OR ?2;");
    fetch.bind(1, "please generate some text");
    fetch.bind(2, "and some more generated text");
    fetch.step();

    SQLite::Column dbIdCol = fetch.column(0);
    SQLite::Column uuidCol = fetch.column(1);
    SQLite::Column sourceCol = fetch.column(2);
    SQLite::Column targetCol = fetch.column(3);
    SQLite::Column statusCol = fetch.column(4);
    SQLite::Column authorityCol = fetch.column(5);

    // TODO

    //ThorQ::Relationship
}

std::shared_ptr<ThorQ::Relationship> ThorQ::Relationship::GetRelationship(int64_t dbId)
{
    // TODO
}
std::shared_ptr<ThorQ::Relationship> ThorQ::Relationship::GetRelationship(uuids::uuid publicId)
{
    // TODO
}
std::shared_ptr<ThorQ::Relationship> ThorQ::Relationship::GetRelationship(std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target)
{
    ThorQ::SQLite::Connection connection("database.db", ThorQ::SQLite::Connection::READONLY);

    if (!connection.isOpen())
    {
        return nullptr;
    }

    connection.execute("INSERT OR IGNORE INTO relationships() VALUES (?1, ?2);");

    // TODO
}

ThorQ::Relationship::Relationship(std::int64_t privateId, uuids::uuid publicId, std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority)
    : m_privId(privateId)
    , m_publicId(publicId)
    , m_source(source)
    , m_target(target)
    , m_mirror(nullptr)
    , m_status(status)
    , m_authority(authority)
{
}

ThorQ::Relationship::~Relationship()
{
    if (m_mirror != nullptr)
    {
        m_mirror->m_mirror = nullptr;
    }
}

uuids::uuid ThorQ::Relationship::publicId() const
{
    return m_publicId;
}

std::shared_ptr<ThorQ::Account> ThorQ::Relationship::source() const
{
    return m_source;
}

std::shared_ptr<ThorQ::Account> ThorQ::Relationship::target() const
{
    return m_target;
}

void ThorQ::Relationship::setMirror(std::shared_ptr<ThorQ::Relationship> mirror)
{
    m_mirror = mirror;
    if (m_mirror != nullptr)
    {
        m_mirror->m_mirror = std::make_shared<ThorQ::Relationship>(this);
    }
}

std::shared_ptr<ThorQ::Relationship> ThorQ::Relationship::mirror() const
{
    return m_mirror;
}

THORQ_RELATIONSHIP_STATUS ThorQ::Relationship::status() const
{
    return m_status.load();
}

THORQ_RELATIONSHIP_AUTHORITY ThorQ::Relationship::authority() const
{
    return m_authority.load();
}

void ThorQ::Relationship::setStatus(THORQ_RELATIONSHIP_STATUS status)
{
    if (m_status.exchange(status) != status)
    {
        //emit statusChanged(status);
    }
}

void ThorQ::Relationship::setAuthority(THORQ_RELATIONSHIP_AUTHORITY authority)
{
    if (m_authority.exchange(authority) != authority)
    {
        //emit authorityChanged(authority);
    }
}
