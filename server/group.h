#ifndef GROUP_H
#define GROUP_H

#include "typedefs_server.h"

#include <uuid.h>

#include <flatbuffers/flatbuffers.h>

#include "typedefs_server.h"

#include <unordered_set>
#include <memory>
#include <shared_mutex>
#include <string>
#include <cstdint>

namespace ThorQ {
class Group
{
    Group(std::int64_t dbId, ThorQ::Uuid id, const std::string& groupname);
public:
    static std::shared_ptr<ThorQ::Account> GetGroup(const std::string& groupname);
    inline static std::shared_ptr<ThorQ::Account> GetGroup(const flatbuffers::String* groupname) { return GetGroup(std::string(groupname->data(), groupname->size())); }
    static std::shared_ptr<ThorQ::Account> NewGroup(const std::string& groupname);
    inline static std::shared_ptr<ThorQ::Account> NewGroup(const flatbuffers::String* groupname) { return NewGroup(std::string(groupname->data(), groupname->size())); }

    enum class Type
    {
        Master,
        FreeForAll
    };


    ThorQ::Uuid id() const { return m_id; }
    std::int64_t dbId() const { return m_dbId; }

    std::string groupname() const;
    bool setGroupname(const std::string& groupname);

    ThorQ::Uuid imageId() const;
    bool setImageId(const ThorQ::Uuid& imageId);

    std::shared_ptr<ThorQ::Account> creator() const;
    bool setCreator(std::shared_ptr<ThorQ::Account> creator);

    bool addMember(std::shared_ptr<ThorQ::Account> member);
    bool removeMember(std::shared_ptr<ThorQ::Account> member);
    bool containsMember(std::shared_ptr<ThorQ::Account> member) const;
    std::vector<std::shared_ptr<ThorQ::Account>> members() const;
private:
    const ThorQ::Uuid m_id;
    const std::int64_t m_dbId;

    std::shared_mutex l_basics;
    std::string m_groupname;
    ThorQ::Uuid m_imageId;
    std::shared_ptr<ThorQ::Account> m_creator;

    std::shared_mutex l_members;
    std::unordered_set<std::shared_ptr<ThorQ::Account>> m_members;
};
}

#endif // GROUP_H
