#include "systemid.h"

#include <cstring>
#include <algorithm>

#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
#include <WinSock2.h>
#elif __linux__
#include <arpa/inet.h>
#endif

typedef quint16 SystemID[5];

constexpr SystemID mask = { 0x4e25, 0xf4a1, 0x5437, 0xab41, 0x0000 };
static SystemID system_id = { 0, 0, 0, 0, 0 };
static bool computed = false;

inline void smear(SystemID id)
{
    for (quint32 i = 0; i < 5; i++)
        for (quint32 j = i; j < 5; j++)
            if (i != j)
				id[i] ^= id[j];

    for (quint32 i = 0; i < 5; i++ )
		id[i] ^= mask[i];
}

inline void unsmear(SystemID id)
{
    for (quint32 i = 0; i < 5; i++)
		id[i] ^= mask[i];

    for (quint32 i = 0; i < 5; i++ )
        for (quint32 j = 0; j < i; j++)
            if (i != j)
				id[4-i] ^= id[4-j];
}

QByteArray ThorQ::systemid_generate()
{
	QByteArray sys_id(ThorQ::SystemID_Internal::getMachineName());
    sys_id.resize(sys_id.size() + std::size(system_id));

	if (!computed)
	{
		memset(system_id, 0, 10);

		system_id[0] = ThorQ::SystemID_Internal::getCpuHash();
		system_id[1] = ThorQ::SystemID_Internal::getVolumeHash();
		ThorQ::SystemID_Internal::getMacHash(system_id[2], system_id[3]);

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

    memcpy(sys_id.end() - std::size(system_id), system_id, std::size(system_id));

	return sys_id;
}

bool ThorQ::systemid_validate(const QByteArray& sys_id)
{
    if (sys_id.size() <= (int)std::size(system_id))
		return false;

	SystemID id;
    memcpy(id, sys_id.end() - std::size(system_id), std::size(system_id));

	unsmear(id);

    quint16 checkSum = 0;
	for (int i = 0; i < 4; i++)
		checkSum += ntohs(id[i]);

	return checkSum == ntohs(id[4]);
}

QString ThorQ::systemid_to_string(QByteArray sys_id)
{
	if (systemid_validate(sys_id))
	{
		SystemID bin_id;
        memcpy(bin_id, sys_id.end() - std::size(system_id), std::size(system_id));

        int nameLen = sys_id.size() - std::size(system_id);
		sys_id.resize(nameLen + 25);

		std::transform(sys_id.begin(), sys_id.begin() + nameLen, sys_id.begin(), ::toupper);

		snprintf(sys_id.begin() + nameLen, 26, "-%04X-%04X-%04X-%04X-%04X", bin_id[0], bin_id[1], bin_id[2], bin_id[3], bin_id[4]);
	}
	else
	{
		sys_id = "INVALID";
	}

	return sys_id;
}
