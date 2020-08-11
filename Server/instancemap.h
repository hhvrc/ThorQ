#ifndef INSTANCEMAP_H
#define INSTANCEMAP_H

#include <set>
#include <map>
#include <vector>

#include "typedefs.h"

namespace ThorQ {
class InstanceMap
{
	struct NameWrapper
	{
		Instance* instance;

		inline explicit operator Instance*() const { return instance; }

		inline bool operator <  (const NameWrapper& comp) const;
		inline bool operator >  (const NameWrapper& comp) const;
		inline bool operator <= (const NameWrapper& comp) const;
		inline bool operator >= (const NameWrapper& comp) const;
		inline bool operator == (const NameWrapper& comp) const;
		inline bool operator != (const NameWrapper& comp) const;
	};
	struct HwidWrapper
	{
		Instance* instance;

		inline explicit operator Instance*() const { return instance; }

		inline bool operator <  (const HwidWrapper& comp) const;
		inline bool operator >  (const HwidWrapper& comp) const;
		inline bool operator <= (const HwidWrapper& comp) const;
		inline bool operator >= (const HwidWrapper& comp) const;
		inline bool operator == (const HwidWrapper& comp) const;
		inline bool operator != (const HwidWrapper& comp) const;
	};

	std::set<NameWrapper> m_nameSorted{};
	std::set<HwidWrapper> m_hwidSorted{};
public:
	InstanceMap() = default;
	~InstanceMap();

	bool tryAdd(Instance* instance);

	Instance* get(const std::string& name);
	Instance* get(const std::vector<std::uint8_t>& hwid);
	bool contains(const std::string& name) const;
	bool contains(const std::vector<std::uint8_t>& hwid) const;
	void remove(const std::string& name);
	void remove(const std::vector<std::uint8_t>& hwid);
	void clear();

	std::vector<Instance*> instances();
};
}

#endif // INSTANCEMAP_H
