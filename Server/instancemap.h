#ifndef INSTANCEMAP_H
#define INSTANCEMAP_H

#include <map>
#include <vector>

namespace ThorQ {
class Instance;
class InstanceMap
{
	std::map<std::string, Instance*> m_internal;
public:
	InstanceMap();
	~InstanceMap();

	bool TryAdd(Instance* peer);

	Instance* Get(const std::string& name);
	bool Contains(const std::string& name) const;
	std::vector<Instance*> ToList();
	void Remove(const std::string& name);

	void Clear();
};
}

#endif // INSTANCEMAP_H
