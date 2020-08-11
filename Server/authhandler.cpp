#include "authhandler.h"

#include <fstream>
#include <set>
#include <ctime>
#include <string>
#include <mutex>
#include <algorithm>
#include <array>

#include "systemid.h"

static std::mutex mutex{};

struct Entry
{
	std::vector<std::uint8_t> sys_id;
	std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN> reg_key;
	time_t last_owner_change = 0;
};

struct EntryPointer_IdComparison_Wrapper
{
	Entry* entryPtr;
};
struct EntryPointer_KeyComparison_Wrapper
{
	Entry* entryPtr;
};

inline bool operator == (const EntryPointer_IdComparison_Wrapper& lhs, const EntryPointer_IdComparison_Wrapper& rhs) { return lhs.entryPtr->sys_id == rhs.entryPtr->sys_id; }
inline bool operator != (const EntryPointer_IdComparison_Wrapper& lhs, const EntryPointer_IdComparison_Wrapper& rhs) { return !(lhs == rhs); }
inline bool operator <  (const EntryPointer_IdComparison_Wrapper& lhs, const EntryPointer_IdComparison_Wrapper& rhs) { return lhs.entryPtr->sys_id < rhs.entryPtr->sys_id; }
inline bool operator >  (const EntryPointer_IdComparison_Wrapper& lhs, const EntryPointer_IdComparison_Wrapper& rhs) { return rhs < lhs; }
inline bool operator <= (const EntryPointer_IdComparison_Wrapper& lhs, const EntryPointer_IdComparison_Wrapper& rhs) { return !(rhs < lhs); }
inline bool operator >= (const EntryPointer_IdComparison_Wrapper& lhs, const EntryPointer_IdComparison_Wrapper& rhs) { return !(lhs < rhs); }

inline bool operator == (const EntryPointer_KeyComparison_Wrapper& lhs, const EntryPointer_KeyComparison_Wrapper& rhs) { return lhs.entryPtr->reg_key == rhs.entryPtr->reg_key; }
inline bool operator != (const EntryPointer_KeyComparison_Wrapper& lhs, const EntryPointer_KeyComparison_Wrapper& rhs) { return !(lhs == rhs); }
inline bool operator <  (const EntryPointer_KeyComparison_Wrapper& lhs, const EntryPointer_KeyComparison_Wrapper& rhs) { return lhs.entryPtr->reg_key < rhs.entryPtr->reg_key; }
inline bool operator >  (const EntryPointer_KeyComparison_Wrapper& lhs, const EntryPointer_KeyComparison_Wrapper& rhs) { return rhs < lhs; }
inline bool operator <= (const EntryPointer_KeyComparison_Wrapper& lhs, const EntryPointer_KeyComparison_Wrapper& rhs) { return !(rhs < lhs); }
inline bool operator >= (const EntryPointer_KeyComparison_Wrapper& lhs, const EntryPointer_KeyComparison_Wrapper& rhs) { return !(lhs < rhs); }

constexpr time_t time_minute =                60;
constexpr time_t time_hour   = time_minute *  60;
constexpr time_t time_day    = time_hour   *  24;
constexpr time_t time_week   = time_day    *   7;
constexpr time_t time_year   = time_day    * 365;
constexpr time_t time_month  = time_year   /  12;

static std::set<EntryPointer_IdComparison_Wrapper> id_set{};
static std::set<EntryPointer_KeyComparison_Wrapper> key_set{};

void Init()
{
}

bool ThorQ::AuthHandler::tryAddRegkey(const std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN>& key)
{
	std::scoped_lock lock(mutex);

	Entry* entry = new Entry{ {}, key, 0 };

	if (key_set.insert({ entry }).second)
		return true;

	delete entry;
	return false;
}

void ThorQ::AuthHandler::removeRegkey(const std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN>& key)
{
	std::scoped_lock lock(mutex);

	Entry temp{ {}, key, 0 };

	// Find it in the key set
	auto key_it = key_set.find({&temp});

	if (key_it != key_set.end())
	{
		// TODO: notify program that [key_it->entryPtr->sys_id] is not unauthorized
		// Delete if its in the key set
		auto id_it = id_set.find({ key_it->entryPtr });
		if (id_it != id_set.end())
			id_set.erase(id_it);

		delete key_it->entryPtr;
		key_set.erase(key_it);
	}
}

ThorQ::AuthHandler::ResponseCode ThorQ::AuthHandler::checkSystemID(const std::vector<std::uint8_t>& sysid)
{
    return REGISTERED;
	std::scoped_lock lock(mutex);

	if (!ThorQ::systemid_validate(sysid))
		return INVALID_SYSTEMID;

	Entry temp{ sysid, {}, 0 };

	// Find it in the id set
	return id_set.find({&temp}) == id_set.end() ? NOT_REGISTERED : REGISTERED;
}

ThorQ::AuthHandler::ResponseCode ThorQ::AuthHandler::tryRegisterSystemID(const std::vector<std::uint8_t>& sysid, const std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN>& regkey)
{
	std::scoped_lock lock(mutex);

	time_t regTime = time(nullptr);

	Entry temp{ sysid, regkey, 0 };

	// Find it in the key set
	auto key_it = key_set.find({&temp});

	// If not found then the regkey is invalid
	if (key_it == key_set.end())
		return INVALID_REGKEY;

	Entry* entry = key_it->entryPtr;

	// If the sys_id matches ours then we are already registered with that regkey
	if (entry->sys_id == sysid)
		return REGISTERED;

	// Re-registration of regkey is rate limited to once a week
	if ((regTime - entry->last_owner_change) < time_week)
		return TIMEOUT;

	// TODO: notify program that [entry->sys_id] is not unauthorized
	entry->sys_id = sysid;
	entry->last_owner_change = regTime;

retry:
	auto id_it = id_set.insert({entry});

	if (!id_it.second)
	{
		id_it.first->entryPtr->sys_id.clear();
		id_set.erase(id_it.first);

		goto retry;
	}

	return REGISTERED;
}
