#ifndef INSTANCEMAP_H
#define INSTANCEMAP_H

#include <map>
#include <vector>

#include "typedefs.h"

namespace ThorQ {
class InstanceMap
{
	std::map<std::string, Instance*> m_internal;
public:
	InstanceMap() = default;
	~InstanceMap();

	bool tryAdd(Instance* peer, const std::string& name);

	Instance* get(const std::string& name);
	bool contains(const std::string& name) const;
	std::vector<Instance*> instances();
	void remove(const std::string& name);

	void clear();
};
}

#endif // INSTANCEMAP_H
