#include "instancemap.h"

#include <algorithm>

#include "instance.h"

bool ThorQ::InstanceMap::NameWrapper::operator <  (const ThorQ::InstanceMap::NameWrapper& comp) const { return instance->name() <  comp.instance->name(); }
bool ThorQ::InstanceMap::NameWrapper::operator >  (const ThorQ::InstanceMap::NameWrapper& comp) const { return instance->name() >  comp.instance->name(); }
bool ThorQ::InstanceMap::NameWrapper::operator <= (const ThorQ::InstanceMap::NameWrapper& comp) const { return instance->name() <= comp.instance->name(); }
bool ThorQ::InstanceMap::NameWrapper::operator >= (const ThorQ::InstanceMap::NameWrapper& comp) const { return instance->name() >= comp.instance->name(); }
bool ThorQ::InstanceMap::NameWrapper::operator == (const ThorQ::InstanceMap::NameWrapper& comp) const { return instance->name() == comp.instance->name(); }
bool ThorQ::InstanceMap::NameWrapper::operator != (const ThorQ::InstanceMap::NameWrapper& comp) const { return instance->name() != comp.instance->name(); }

bool ThorQ::InstanceMap::HwidWrapper::operator <  (const ThorQ::InstanceMap::HwidWrapper& comp) const { return instance->hwid() <  comp.instance->hwid(); }
bool ThorQ::InstanceMap::HwidWrapper::operator >  (const ThorQ::InstanceMap::HwidWrapper& comp) const { return instance->hwid() >  comp.instance->hwid(); }
bool ThorQ::InstanceMap::HwidWrapper::operator <= (const ThorQ::InstanceMap::HwidWrapper& comp) const { return instance->hwid() <= comp.instance->hwid(); }
bool ThorQ::InstanceMap::HwidWrapper::operator >= (const ThorQ::InstanceMap::HwidWrapper& comp) const { return instance->hwid() >= comp.instance->hwid(); }
bool ThorQ::InstanceMap::HwidWrapper::operator == (const ThorQ::InstanceMap::HwidWrapper& comp) const { return instance->hwid() == comp.instance->hwid(); }
bool ThorQ::InstanceMap::HwidWrapper::operator != (const ThorQ::InstanceMap::HwidWrapper& comp) const { return instance->hwid() != comp.instance->hwid(); }


ThorQ::InstanceMap::~InstanceMap()
{
	m_hwidSorted.clear();
	m_nameSorted.clear();
}

bool ThorQ::InstanceMap::tryAdd(Instance* instance)
{
	auto it_name = m_nameSorted.insert({instance});
	auto it_hwid = m_hwidSorted.insert({instance});

	if (it_name.second && it_hwid.second)
	{
		return true;
	}

	if (it_name.second)
	{
		m_nameSorted.erase(it_name.first);
	}

	if (it_hwid.second)
	{
		m_hwidSorted.erase(it_hwid.first);
	}

	return false;
}

ThorQ::Instance* ThorQ::InstanceMap::get(const std::string& name)
{
	auto it = std::find_if(m_nameSorted.begin(), m_nameSorted.end(), [&](const NameWrapper& w){ return w.instance->name() == name; });

	if (it != m_nameSorted.end())
		return it->instance;

	return nullptr;
}

ThorQ::Instance* ThorQ::InstanceMap::get(const std::vector<uint8_t>& hwid)
{
	auto it = std::find_if(m_hwidSorted.begin(), m_hwidSorted.end(), [&](const HwidWrapper& w){ return w.instance->hwid() == hwid; });

	if (it != m_hwidSorted.end())
		return it->instance;

	return nullptr;
}

bool ThorQ::InstanceMap::contains(const std::string& name) const
{
	return std::find_if(m_nameSorted.begin(), m_nameSorted.end(), [&](const NameWrapper& w){ return w.instance->name() == name; }) != m_nameSorted.end();
}

bool ThorQ::InstanceMap::contains(const std::vector<uint8_t>& hwid) const
{
	return std::find_if(m_hwidSorted.begin(), m_hwidSorted.end(), [&](const HwidWrapper& w){ return w.instance->hwid() == hwid; }) != m_hwidSorted.end();
}

void ThorQ::InstanceMap::remove(const std::string& name)
{
	auto it = std::find_if(m_nameSorted.begin(), m_nameSorted.end(), [&](const NameWrapper& w){ return w.instance->name() == name; });

	if (it != m_nameSorted.end())
		m_nameSorted.erase(it);
}

void ThorQ::InstanceMap::remove(const std::vector<uint8_t>& hwid)
{
	auto it = std::find_if(m_hwidSorted.begin(), m_hwidSorted.end(), [&](const HwidWrapper& w){ return w.instance->hwid() == hwid; });

	if (it != m_hwidSorted.end())
		m_hwidSorted.erase(it);
}

void ThorQ::InstanceMap::clear()
{
	m_nameSorted.clear();
	m_hwidSorted.clear();
}

std::vector<ThorQ::Instance*> ThorQ::InstanceMap::instances()
{
	return std::vector<Instance*>(m_nameSorted.begin(), m_nameSorted.end());
}
