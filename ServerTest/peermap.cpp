#include "peermap.h"

using namespace ThorQ;

InstanceMap::InstanceMap()
{

}

InstanceMap::~InstanceMap()
{

}

bool InstanceMap::TryAdd(const std::string& name, Instance* peer)
{
	auto it = m_internal.find(name);

	if (it != m_internal.end())
	{
		return false;
	}
	m_internal.insert(std::pair<std::string, Instance*>(name, peer));
	return true;
}

Instance* InstanceMap::GetInstance(const std::string& name)
{
	auto it = m_internal.find(name);

	if (it != m_internal.end())
		return it->second;
	return nullptr;
}

std::string InstanceMap::GetName(const Instance* peer)
{
	for (auto it = m_internal.begin(); it != m_internal.end(); it++)
		if (it->second == peer)
			return it->first;
	return std::string();
}

bool InstanceMap::ContainsInstance(const Instance* peer) const
{
	for (auto it = m_internal.begin(); it != m_internal.end(); it++)
		if (it->second == peer)
			return true;
	return false;
}

bool InstanceMap::ContainsName(const std::string& name) const
{
	return m_internal.find(name) != m_internal.end();
}

std::vector<std::string> InstanceMap::GetNames()
{
	std::vector<std::string> names;
	names.reserve(m_internal.size());
	for (auto it = m_internal.begin(); it != m_internal.end(); it++)
		names.push_back(it->first);
	return names;
}

std::vector<Instance*> InstanceMap::GetInstances()
{
	std::vector<Instance*> peers;
	peers.reserve(m_internal.size());
	for (auto it = m_internal.begin(); it != m_internal.end(); it++)
		peers.push_back(it->second);
	return peers;
}

void InstanceMap::Remove(const Instance* peer)
{
	for (auto it = m_internal.begin(); it != m_internal.end(); it++)
		if (it->second == peer)
			m_internal.erase(it);
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
