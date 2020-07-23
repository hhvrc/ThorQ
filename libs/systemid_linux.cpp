#include "systemid.h"

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <cstring>
#include <cassert>

#include <linux/if.h>
#include <linux/sockios.h>

#include <sys/resource.h>
#include <sys/utsname.h>

//---------------------------------get MAC addresses ---------------------------------
// we just need this for purposes of unique machine id. So any one or two
// mac's is fine.
std::uint16_t hashMacAddress(std::uint8_t* mac)
{
	std::uint16_t hash = 0;

	for (int i = 0; i < 6; i++)
	{
		hash += ( mac[i] << (( i & 1 ) * 8 ));
	}
	return hash;
}

void ThorQ::SystemID_Internal::getMacHash(std::uint16_t& mac1, std::uint16_t& mac2)
{
	mac1 = 0;
	mac2 = 0;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP );
	if (sock < 0) return;

	// enumerate all IP addresses of the system
	struct ifconf conf;
	char ifconfbuf[ 128 * sizeof(struct ifreq)  ];
	memset( ifconfbuf, 0, sizeof( ifconfbuf ));
	conf.ifc_buf = ifconfbuf;
	conf.ifc_len = sizeof( ifconfbuf );
	if ( ioctl( sock, SIOCGIFCONF, &conf ))
		return;

	// get MAC address
	bool foundMac1 = false;
	struct ifreq* ifr;
	for ( ifr = conf.ifc_req; (std::int8_t*)ifr < (std::int8_t*)conf.ifc_req + conf.ifc_len; ifr++ )
	{
		if ( ifr->ifr_addr.sa_data == (ifr+1)->ifr_addr.sa_data )
			continue;  // duplicate, skip it

		if ( ioctl( sock, SIOCGIFFLAGS, ifr ))
			continue;  // failed to get flags, skip it
		if ( ioctl( sock, SIOCGIFHWADDR, ifr ) == 0 )
		{
			if ( !foundMac1 )
			{
				foundMac1 = true;
				mac1 = hashMacAddress( (std::uint8_t*)&(ifr->ifr_addr.sa_data));
			} else {
				mac2 = hashMacAddress( (std::uint8_t*)&(ifr->ifr_addr.sa_data));
				break;
			}
		}
	}

	close( sock );

	// sort the mac addresses. We don't want to invalidate
	// both macs if they just change order.
	if ( mac1 > mac2 )
	{
		std::uint16_t tmp = mac2;
		mac2 = mac1;
		mac1 = tmp;
	}
}

std::uint16_t ThorQ::SystemID_Internal::getVolumeHash()
{
	// we don't have a 'volume serial number' like on windows.
	// Lets hash the system name instead.
	std::uint8_t* sysname = (std::uint8_t*)ThorQ::SystemID_Internal::getMachineName();

	std::uint16_t hash = 0;
	for (int i = 0; sysname[i] != 0; i++)
		hash += (sysname[i] << (( i & 1 ) * 8 ));

	return hash;
}

static inline void getCpuid( std::uint32_t* p, std::uint32_t ax )
{
	__asm __volatile(
				"movl %%ebx, %%esi\n\t"
				"cpuid\n\t"
				"xchgl %%ebx, %%esi"
				: "=a" (p[0])
			, "=S" (p[1])
			, "=c" (p[2])
			, "=d" (p[3])
		: "0" (ax)
		);
}

std::uint16_t ThorQ::SystemID_Internal::getCpuHash()
{
	std::uint32_t cpuinfo[4]{0};
	getCpuid(cpuinfo, 0);

	std::uint16_t hash = 0;
	for (int i = 0; i < 4; i++)
		hash += (cpuinfo[i] & 0xFFFF) + (cpuinfo[i] >> 16);

	return hash;
}

const char* ThorQ::SystemID_Internal::getMachineName()
{
	static struct utsname u;

	if (uname(&u) < 0)
		return "unknown";

	return u.nodename;
}
