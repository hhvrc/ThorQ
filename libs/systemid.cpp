#include "systemid.h"

#include "cpuid.h"

#include <cstring>
#include <algorithm>

#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
#include <WinSock2.h>
#elif __linux__
#include <arpa/inet.h>
#endif

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/compile.h>

typedef std::uint16_t SysHID[5];

constexpr SysHID mask = { 0x4e25, 0xf4a1, 0x5437, 0xab41, 0x0000 };
static SysHID sysHid = { 0, 0, 0, 0, 0 };
static bool computed = false;

inline void smear(SysHID id)
{
    for (std::uint32_t i = 0; i < 5; i++)
        for (std::uint32_t j = i; j < 5; j++)
            if (i != j)
				id[i] ^= id[j];

    for (std::uint32_t i = 0; i < 5; i++ )
		id[i] ^= mask[i];
}

inline void unsmear(SysHID id)
{
    for (std::uint32_t i = 0; i < 5; i++)
		id[i] ^= mask[i];

    for (std::uint32_t i = 0; i < 5; i++ )
        for (std::uint32_t j = 0; j < i; j++)
            if (i != j)
				id[4-i] ^= id[4-j];
}

std::vector<std::uint8_t> ThorQ::SystemID::systemid_generate()
{
    std::string machineName = ThorQ::SystemID::Internal::getMachineName();

    std::vector<std::uint8_t> sys_id;
    sys_id.resize(machineName.size() + sizeof(SysHID));

    std::memcpy(sys_id.data(), machineName.data(), machineName.size());

    if (!computed)
	{
        std::memset(sysHid, 0, sizeof(SysHID));

        sysHid[0] = ThorQ::SystemID::Internal::getCpuHash();
        sysHid[1] = ThorQ::SystemID::Internal::getVolumeHash();
        ThorQ::SystemID::Internal::getMacHash(sysHid[2], sysHid[3]);

		for (int i = 0; i < 4; i++)
		{
			// fifth block is some checksum
            sysHid[4] += sysHid[i];

			// convert to network order
            sysHid[i] = htons(sysHid[i]);
		}

        sysHid[4] = htons(sysHid[4]);

        smear(sysHid);

		computed = true;
	}

    std::memcpy(sys_id.data() + machineName.size(), sysHid, sizeof(SysHID));

	return sys_id;
}

bool ThorQ::SystemID::systemid_validate(std::span<std::uint8_t> sys_id)
{
    if (sys_id.size() <= (int)std::size(sysHid))
		return false;

    SysHID hid;
    std::memcpy(hid, sys_id.data() + sys_id.size() - sizeof(SysHID), sizeof(SysHID));

    unsmear(hid);

    std::uint16_t checkSum = 0;
	for (int i = 0; i < 4; i++)
        checkSum += ntohs(hid[i]);

    return checkSum == ntohs(hid[4]);
}

std::string ThorQ::SystemID::systemid_to_string(std::span<std::uint8_t> bin_id)
{
    std::string str_id;

    if (systemid_validate(bin_id))
    {
        std::size_t nameLen = bin_id.size() - sizeof(SysHID);

        const SysHID* hid = reinterpret_cast<const SysHID*>(bin_id.data() + nameLen);

        std::string name(nameLen, ' ');
        std::transform(bin_id.begin(), bin_id.begin() + nameLen, name.begin(), ::toupper);

        str_id = fmt::format(FMT_COMPILE("{}-{:04X}-{:04X}-{:04X}-{:04X}-{:04X}"), name, (*hid)[0], (*hid)[1], (*hid)[2], (*hid)[3], (*hid)[4]);
	}
	else
	{
        str_id = "INVALID";
	}

    return str_id;
}

std::uint16_t ThorQ::SystemID::Internal::getCpuHash()
{
    int cpuinfo[4] { 0, 0, 0, 0 };
    __cpuid(cpuinfo, 0);
    std::uint16_t hash = 0;
    for (int i = 0; i < 4; i++) {
        hash += (cpuinfo[i] & 0xFFFF) + (cpuinfo[i] >> 16);
    }

    return hash;
}
