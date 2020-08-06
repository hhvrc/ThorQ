#include "instancemap.h"

#include "instance.h"

ThorQ::InstanceMap::~InstanceMap() { m_internal.clear(); }

bool ThorQ::InstanceMap::TryAdd(Instance* peer, const std::string& name)
{
    auto it = m_internal.find(name);

    if (it != m_internal.end())
        return false;

	m_internal.insert(std::pair<std::string, Instance*>(peer->name(), peer));
	return true;
}

ThorQ::Instance* ThorQ::InstanceMap::Get(const std::string& name)
{
	auto it = m_internal.find(name);

	if (it != m_internal.end())
		return it->second;
	return nullptr;
}

bool ThorQ::InstanceMap::Contains(const std::string& name) const
{
	return m_internal.find(name) != m_internal.end();
}

std::vector<ThorQ::Instance*> ThorQ::InstanceMap::ToList()
{
	std::vector<Instance*> peers;
	peers.reserve(m_internal.size());
	for (auto &peer : m_internal)
		peers.push_back(peer.second);
	return peers;
}

void ThorQ::InstanceMap::Remove(const std::string& name)
{
	auto it = m_internal.find(name);

	if (it != m_internal.end())
		m_internal.erase(it);
}

void ThorQ::InstanceMap::Clear()
{
	m_internal.clear();
}
