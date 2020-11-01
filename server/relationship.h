#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <atomic>
#include <memory>

#include <enums.h>

#include "uuid.h"
#include "typedefs_server.h"

namespace ThorQ {
class Relationship // An attributed "edge" in a labeled graph
{
    Relationship(ThorQ::Uuid publicId, std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority);
public:
    static std::shared_ptr<ThorQ::Relationship> GetRelationship(ThorQ::Uuid publicId);
    static std::shared_ptr<ThorQ::Relationship> GetRelationship(std::shared_ptr<ThorQ::Account> source, std::shared_ptr<ThorQ::Account> target);
    ~Relationship();

    ThorQ::Uuid publicId() const;

    std::shared_ptr<ThorQ::Account> source() const;
    std::shared_ptr<ThorQ::Account> target() const;

    static void setMirrors(std::shared_ptr<ThorQ::Relationship> relationship1, std::shared_ptr<ThorQ::Relationship> relationship2);
    std::shared_ptr<ThorQ::Relationship> mirror() const; // Mirror of this

    THORQ_RELATIONSHIP_STATUS status() const;
    void setStatus(THORQ_RELATIONSHIP_STATUS status);

    THORQ_RELATIONSHIP_AUTHORITY authority() const;
    void setAuthority(THORQ_RELATIONSHIP_AUTHORITY authority);
private:
    const ThorQ::Uuid  m_publicId;
    const std::shared_ptr<ThorQ::Account> m_source;
    const std::shared_ptr<ThorQ::Account> m_target;
    std::shared_ptr<ThorQ::Relationship>  m_mirror;
    std::atomic<THORQ_RELATIONSHIP_STATUS>     m_status;
    std::atomic<THORQ_RELATIONSHIP_AUTHORITY>  m_authority;
};
}

#endif // RELATIONSHIP_H
