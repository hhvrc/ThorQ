#include "instancemap.h"

#include "instance.h"

ThorQ::InstanceMap::~InstanceMap() { m_internal.clear(); }

bool ThorQ::InstanceMap::tryAdd(Instance* peer, const std::string& name)
{
    auto it = m_internal.find(name);

	if (it == m_internal.end())
	{
		m_internal.insert(std::pair<std::string, Instance*>(peer->name(), peer));
		return true;
	}

	return false;
}

ThorQ::Instance* ThorQ::InstanceMap::get(const std::string& name)
{
	auto it = m_internal.find(name);

	if (it == m_internal.end())
	{
		return nullptr;
	}

	return it->second;
}

bool ThorQ::InstanceMap::contains(const std::string& name) const
{
	return m_internal.find(name) != m_internal.end();
}

std::vector<ThorQ::Instance*> ThorQ::InstanceMap::instances()
{
	std::vector<Instance*> peers;
	peers.reserve(m_internal.size());

	for (auto &peer : m_internal)
		peers.push_back(peer.second);

	return peers;
}

void ThorQ::InstanceMap::remove(const std::string& name)
{
	auto it = m_internal.find(name);

	if (it != m_internal.end())
		m_internal.erase(it);
}

void ThorQ::InstanceMap::clear()
{
	m_internal.clear();
}
