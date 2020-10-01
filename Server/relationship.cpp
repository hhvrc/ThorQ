#include "relationship.h"

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
