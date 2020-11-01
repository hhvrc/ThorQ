#include "relationship.h"

#include "lsql/transaction.h"
#include "lsql/connection.h"
#include "lsql/column.h"
#include "lsql/query.h"

#include "uuid.h"
#include "account.h"

std::shared_mutex g_relationships_lock;
std::unordered_map<ThorQ::Uuid, std::shared_ptr<ThorQ::Relationship>> g_relationships;

std::shared_ptr<ThorQ::Relationship> ThorQ::Relationship::GetRelationship(ThorQ::Uuid publicId)
{
    // TODO
    return nullptr;
}
std::shared_ptr<ThorQ::Relationship> ThorQ::Relationship::GetRelationship(std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target)
{
    LSql::Connection connection("database.db", LSql::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return nullptr;
    }

    LSql::Transaction transaction = connection.transaction();

    LSql::Query query = connection.query(
                "INSERT OR IGNORE INTO relationships(source, target, uuid, status, authority) VALUES (:1, :2, :3, :5, :6),(:2, :1, :4, :5, :6);"
                "SELECT source, uuid, status, authority FROM relationships WHERE source LIKE :1 OR :2 AND target LIKE :1 OR :2 LIMIT 2;"
                );
    query.bind(1, source->databaseId());
    query.bind(2, target->databaseId());
    query.bind(3, ThorQ::Uuid::NewUuid().toString());
    query.bind(4, ThorQ::Uuid::NewUuid().toString());
    query.bind(5, (int32_t)THORQ_RELATIONSHIP_STATUS::NONE);
    query.bind(6, (int32_t)THORQ_RELATIONSHIP_AUTHORITY::SILENT);
    query.step();

    std::shared_ptr<ThorQ::Relationship> sourceRelationship;
    std::shared_ptr<ThorQ::Relationship> targetRelationship;

    for (int i = 0; i < 2; i++)
    {
        if (!query.step() && query.columnCount() != 4)
        {
            return nullptr;
        }

        std::int64_t sourceId = query.column(0).getInt64();

        ThorQ::Uuid uuid;
        if (!ThorQ::Uuid::TryParse(query.column(1).getText(), uuid))
        {
            return nullptr;
        }

        std::int32_t status    = query.column(2).getInt();
        std::int32_t authority = query.column(3).getInt();

        if (sourceId == source->databaseId())
        {
            sourceRelationship = std::shared_ptr<ThorQ::Relationship>(new Relationship(uuid, source, target, (THORQ_RELATIONSHIP_STATUS)status, (THORQ_RELATIONSHIP_AUTHORITY)authority));
            //sourceRelationship = std::shared_ptr<ThorQ::Relationship>(new Relationship(accountId, uuid.value(), source, target, (THORQ_RELATIONSHIP_STATUS)status, (THORQ_RELATIONSHIP_AUTHORITY)authority));
        }
        else if (sourceId == target->databaseId())
        {
            targetRelationship = std::shared_ptr<ThorQ::Relationship>(new Relationship(uuid, target, source, (THORQ_RELATIONSHIP_STATUS)status, (THORQ_RELATIONSHIP_AUTHORITY)authority));
            //targetRelationship = std::shared_ptr<ThorQ::Relationship>(new Relationship(accountId, uuid.value(), target, source, (THORQ_RELATIONSHIP_STATUS)status, (THORQ_RELATIONSHIP_AUTHORITY)authority));
        }
        else
        {
            return nullptr;
        }
    }

    std::unique_lock l(g_relationships_lock);
    g_relationships.insert(std::pair<ThorQ::Uuid, std::shared_ptr<ThorQ::Relationship>>(sourceRelationship->publicId(), sourceRelationship));
    // TODO

    //ThorQ::Relationship
}

ThorQ::Relationship::Relationship(ThorQ::Uuid publicId, std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority)
    : m_publicId(publicId)
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

ThorQ::Uuid ThorQ::Relationship::publicId() const
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

void ThorQ::Relationship::setMirrors(std::shared_ptr<ThorQ::Relationship> relationship1, std::shared_ptr<ThorQ::Relationship> relationship2)
{
    if (relationship1 != nullptr && relationship2 != nullptr)
    {
        relationship1->m_mirror = relationship2;
        relationship2->m_mirror = relationship1;
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
