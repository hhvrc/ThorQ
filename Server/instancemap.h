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

		bool TryAdd(const std::string& name, Instance* peer);

		Instance* GetInstance(const std::string& name);
		std::string GetName(const Instance* peer);

		bool ContainsInstance(const Instance* peer) const;
		bool ContainsName(const std::string& name) const;

		std::vector<std::string> GetNames();
		std::vector<Instance*> GetInstances();

		void Remove(const Instance* peer);
		void Remove(const std::string& name);

		void Clear();
	};
}

#endif // PEERMAP_H
