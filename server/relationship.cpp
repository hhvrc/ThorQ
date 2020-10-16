#include "relationship.h"

#include "sqlite/connection.h"

ThorQ::Relationship* ThorQ::Relationship::NewRelationship(ThorQ::Account* account1, ThorQ::Account* account2)
{
    auto optional = ThorQ::SQLite::Connection::Open("database.db", ThorQ::SQLite::Connection::READWRITE);

    if (!optional.has_value())
    {
        return nullptr;
    }

    ThorQ::SQLite::Connection con = optional.value();

    con.execute("INSERT OR IGNORE INTO relationships() VALUES (?1, ?2);");
}

ThorQ::Relationship *ThorQ::Relationship::GetRelationship(int64_t dbId)
{

}
ThorQ::Relationship *ThorQ::Relationship::GetRelationship(uuids::uuid publicId)
{

}
ThorQ::Relationship* ThorQ::Relationship::GetRelationship(ThorQ::Account *account1, ThorQ::Account *account2)
{
    auto optional = ThorQ::SQLite::Connection::Open("database.db", ThorQ::SQLite::Connection::READONLY);

    if (!optional.has_value())
    {
        return nullptr;
    }

    con.execute("INSERT OR IGNORE INTO relationships() VALUES (?1, ?2);");
}

ThorQ::Relationship::Relationship(std::uint64_t privateId, uuids::uuid publicId, ThorQ::Account* source, ThorQ::Account* target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority)
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

ThorQ::Account *ThorQ::Relationship::source() const
{
    return m_source;
}

ThorQ::Account *ThorQ::Relationship::target() const
{
    return m_target;
}

void ThorQ::Relationship::setMirror(ThorQ::Relationship* mirror)
{
    m_mirror = mirror;
    if (m_mirror != nullptr)
    {
        m_mirror->m_mirror = this;
    }
}

ThorQ::Relationship *ThorQ::Relationship::mirror() const
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
        emit statusChanged(status);
    }
}

void ThorQ::Relationship::setAuthority(THORQ_RELATIONSHIP_AUTHORITY authority)
{
    if (m_authority.exchange(authority) != authority)
    {
        emit authorityChanged(authority);
    }
}

void ThorQ::Relationship::UpdateDatabase()
{

}
