#include "instancemap.h"

using namespace ThorQ;

InstanceMap::InstanceMap() : m_internal() {}

InstanceMap::~InstanceMap() { m_internal.clear(); }

bool InstanceMap::TryAdd(Instance* peer)
{
	if (!peer->hasName())
		return false;

	auto it = m_internal.find(peer->name());

	if (it != m_internal.end())
	{
		return false;
	}

	m_internal.insert(std::pair<std::string, Instance*>(peer->name(), peer));
	return true;
}

Instance* InstanceMap::Get(const std::string& name)
{
	auto it = m_internal.find(name);

	if (it != m_internal.end())
		return it->second;
	return nullptr;
}

bool InstanceMap::Contains(const std::string& name) const
{
	return m_internal.find(name) != m_internal.end();
}

std::vector<Instance*> InstanceMap::ToList()
{
	std::vector<Instance*> peers;
	peers.reserve(m_internal.size());
	for (auto it = m_internal.begin(); it != m_internal.end(); it++)
		peers.push_back(it->second);
	return peers;
}

void InstanceMap::Remove(const std::string& name)
{
	auto it = m_internal.find(name);

	if (it != m_internal.end())
		m_internal.erase(it);
}

void InstanceMap::Clear()
{
	m_internal.clear();
}
