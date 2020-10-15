#include "relationship.h"

ThorQ::Relationship::Relationship(int privateId, uuids::uuid publicId, ThorQ::Account *source, ThorQ::Account *target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority)
    : m_privId(privateId)
    , m_publicId(publicId)
    , m_source(source)
    , m_target(target)
    , m_status(status)
    , m_authority(authority)
{
}

ThorQ::Relationship::~Relationship()
{

}

uuids::uuid ThorQ::Relationship::publicId() const
{
    return m_publicId;
}

ThorQ::Account* ThorQ::Relationship::source() const
{
    return m_source;
}

ThorQ::Account *ThorQ::Relationship::target() const
{
    return m_target;
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
