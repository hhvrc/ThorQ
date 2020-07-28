#ifndef PEERMAP_H
#define PEERMAP_H

#include <map>
#include <vector>

#include "instance.h"

namespace ThorQ {
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

#endif // PEERMAP_H
