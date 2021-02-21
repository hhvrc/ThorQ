#include "group.h"

#include <mutex>

ThorQ::Group::Group(std::int64_t dbId, ThorQ::Uuid id, const std::string& groupname)
    : m_id(id)
    , m_dbId(dbId)
    , m_groupname(groupname)
{
}

std::shared_ptr<ThorQ::Account> ThorQ::Group::GetGroup(const std::string& groupname)
{
    return {};
}

std::shared_ptr<ThorQ::Account> ThorQ::Group::NewGroup(const std::string& groupname)
{
    return {};
}

std::string ThorQ::Group::groupname() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_basics));
    return m_groupname;
}

bool ThorQ::Group::setGroupname(const std::string& groupname)
{
    std::unique_lock l(l_basics);
    if (m_groupname == groupname) {
        return true;
    }
}

ThorQ::Uuid ThorQ::Group::imageId() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_basics));
    return m_imageId;
}

bool ThorQ::Group::setImageId(const ThorQ::Uuid& imageId)
{
    std::unique_lock l(l_basics);
    if (m_imageId == imageId) {
        return true;
    }
}

std::shared_ptr<ThorQ::Account> ThorQ::Group::creator() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_basics));
    return m_creator;
}

bool ThorQ::Group::setCreator(std::shared_ptr<ThorQ::Account> creator)
{
    std::unique_lock l(l_basics);
    if (creator == nullptr) {
        return false;
    }
    if (m_creator == creator) {
        return true;
    }

}

bool ThorQ::Group::addMember(std::shared_ptr<ThorQ::Account> member)
{
    std::unique_lock l(l_members);

    // DB stuff

    if (!m_members.insert(member).second) {
        return false;
    }

    // DB commit

    return true;
}

bool ThorQ::Group::removeMember(std::shared_ptr<ThorQ::Account> member)
{
    std::unique_lock l(l_members);

    // DB stuff

    auto it = m_members.find(member);
    if (it == m_members.end()) {
        return false;
    }
    m_members.erase(it);

    // DB commit

    return true;
}

bool ThorQ::Group::containsMember(std::shared_ptr<ThorQ::Account> member) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_members));
    return m_members.contains(member);
}

std::vector<std::shared_ptr<ThorQ::Account>> ThorQ::Group::members() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_members));
    return std::vector<std::shared_ptr<ThorQ::Account>>(m_members.begin(), m_members.end());
}
