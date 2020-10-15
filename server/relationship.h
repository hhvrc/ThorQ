#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <atomic>

#include <enums.h>
#include <uuid.h>

#include "typedefs_server.h"

namespace ThorQ {
class Relationship
{
public:
    Relationship(int privateId, uuids::uuid publicId, Account* target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority);
    ~Relationship();

    uuids::uuid publicId() const;

    Account* source() const;
    Account* target() const;
    Relationship* mirror() const;

    THORQ_RELATIONSHIP_STATUS status() const;
    void setStatus(THORQ_RELATIONSHIP_STATUS status);

    THORQ_RELATIONSHIP_AUTHORITY authority() const;
    void setAuthority(THORQ_RELATIONSHIP_AUTHORITY authority);
private:
    int m_privId;
    uuids::uuid m_publicId;
    Account* m_source;
    Account* m_target;
    Relationship* m_mirror;
    std::atomic<THORQ_RELATIONSHIP_STATUS>    m_status;
    std::atomic<THORQ_RELATIONSHIP_AUTHORITY> m_authority;
};
}

#endif // RELATIONSHIP_H
