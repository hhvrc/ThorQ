#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <atomic>
#include <memory>
#include <uuid.h>

#include <enums.h>
#include <stduuid/include/uuid.h>

#include "typedefs_server.h"

namespace ThorQ {
class Relationship // An attributed "edge" in a labeled graph
{
public:
    static std::shared_ptr<ThorQ::Relationship> NewRelationship(std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target);
    static std::shared_ptr<ThorQ::Relationship> GetRelationship(std::int64_t dbId);
    static std::shared_ptr<ThorQ::Relationship> GetRelationship(uuids::uuid publicId);
    static std::shared_ptr<ThorQ::Relationship> GetRelationship(std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target);
    Relationship(std::uint64_t privateId, uuids::uuid publicId, std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority);
    ~Relationship();

    uuids::uuid publicId() const;

    std::shared_ptr<ThorQ::Account> source() const;
    std::shared_ptr<ThorQ::Account> target() const;

    void setMirror(std::shared_ptr<ThorQ::Relationship> mirror);
    std::shared_ptr<ThorQ::Relationship> mirror() const; // Mirror of this

    THORQ_RELATIONSHIP_STATUS status() const;
    void setStatus(THORQ_RELATIONSHIP_STATUS status);

    THORQ_RELATIONSHIP_AUTHORITY authority() const;
    void setAuthority(THORQ_RELATIONSHIP_AUTHORITY authority);
protected:
    void UpdateDatabase();
private:
    const std::int64_t m_privId;
    const uuids::uuid  m_publicId;
    const std::shared_ptr<ThorQ::Account> m_source;
    const std::shared_ptr<ThorQ::Account> m_target;
    std::shared_ptr<ThorQ::Relationship> m_mirror;
    std::atomic<THORQ_RELATIONSHIP_STATUS>     m_status;
    std::atomic<THORQ_RELATIONSHIP_AUTHORITY>  m_authority;
};
}

#endif // RELATIONSHIP_H
