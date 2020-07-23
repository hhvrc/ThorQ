#include "systemid.h"

#include <string>
#include <cstring>
#include <algorithm>
#include <arpa/inet.h>

using namespace ThorQ::SystemID_Internal;

constexpr std::uint16_t mask[5] = { 0x4e25, 0xf4a1, 0x5437, 0xab41, 0x0000 };
static std::uint16_t system_id[5] = { 0, 0, 0, 0, 0 };
static bool computed = false;

inline void smear(std::uint16_t* id)
{
	for ( std::uint32_t i = 0; i < 5; i++ )
		for ( std::uint32_t j = i; j < 5; j++ )
			if ( i != j )
				id[i] ^= id[j];

	for ( std::uint32_t i = 0; i < 5; i++ )
		id[i] ^= mask[i];
}

inline void unsmear(std::uint16_t* id)
{
	for ( std::uint32_t i = 0; i < 5; i++ )
		id[i] ^= mask[i];

	for ( std::uint32_t i = 0; i < 5; i++ )
		for ( std::uint32_t j = 0; j < i; j++ )
			if ( i != j )
				id[4-i] ^= id[4-j];
}

std::vector<std::uint8_t> systemid_generate()
{
	std::vector<std::uint8_t> suid;

	{
		if (!computed)
		{
			memset(system_id, 0, 10);

			system_id[0] = getCpuHash();
			system_id[1] = getVolumeHash();
			getMacHash(system_id[2], system_id[3]);

			for (int i = 0; i < 4; i++)
			{
				// fifth block is some checksum
				system_id[4] += system_id[i];

				// convert to network order
				system_id[i] = htons(system_id[i]);
			}

			system_id[4] = htons(system_id[4]);

			smear(system_id);

			computed = true;
		}

		suid.resize(sizeof(system_id));
		memcpy(suid.data(), system_id, sizeof(system_id));
	}

	{
		// get the name of the computer
		std::string name = getMachineName();
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);

		suid.insert(suid.begin() + 10, name.begin(), name.end());
	}

	return suid;
}

bool systemid_validate(const std::vector<std::uint8_t>& suid)
{
	if (suid.size() <= 10)
		return false;

	std::uint16_t id[5];
	memcpy(id, suid.data(), 10);

	unsmear(id);

	std::uint16_t checkSum = 0;
	for (int i = 0; i < 4; i++)
		checkSum += ntohs(id[i]);

	return checkSum == ntohs(id[4]);
}

std::string systemid_to_string(const std::vector<std::uint8_t>& suid)
{
	std::uint16_t bin_id[5] = { 0, 0, 0, 0, 0 };
	std::string str_id;

	if (systemid_validate(suid))
	{
		memcpy(bin_id, suid.data(), 10);
		str_id.insert(str_id.begin(), suid.begin() + 10, suid.end());
	}
	else
	{
		str_id = "INVALID";
	}

	std::size_t strSize = str_id.size();
	str_id.resize(strSize + 26);
	snprintf(str_id.data() + strSize, 26, "-%04X-%04X-%04X-%04X-%04X", bin_id[0], bin_id[1], bin_id[2], bin_id[3], bin_id[4]);

	return str_id;
}
