#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <atomic>

#include <enums.h>
#include <stduuid/include/uuid.h>

#include "typedefs_server.h"

namespace ThorQ {
class Relationship // An attributed "edge" in a labeled graph
{
public:
    static Relationship* NetRelationship(Account* account1, Account* account2);
    static Relationship* GetRelationship(std::int64_t dbId);
    static Relationship* GetRelationship(uuids::uuid publicId);
    static Relationship* GetRelationship(Account* source, Account* target);
    Relationship(std::uint64_t privateId, uuids::uuid publicId, Account* source, Account* target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority);
    ~Relationship();

    uuids::uuid publicId() const;

    Account* source() const;
    Account* target() const;

    void setMirror(Relationship* mirror);
    Relationship* mirror() const; // Mirror of this

    THORQ_RELATIONSHIP_STATUS status() const;
    void setStatus(THORQ_RELATIONSHIP_STATUS status);

    THORQ_RELATIONSHIP_AUTHORITY authority() const;
    void setAuthority(THORQ_RELATIONSHIP_AUTHORITY authority);
protected:
    void UpdateDatabase();
private:
    std::int64_t  m_privId;
    uuids::uuid   m_publicId;
    Account*      m_source;
    Account*      m_target;
    Relationship* m_mirror;
    std::atomic<THORQ_RELATIONSHIP_STATUS>    m_status;
    std::atomic<THORQ_RELATIONSHIP_AUTHORITY> m_authority;
};
}

#endif // RELATIONSHIP_H
