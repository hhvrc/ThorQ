#include "systemid.h"

#include <windows.h>
#include <intrin.h>
#include <iphlpapi.h>

// we just need this for purposes of unique machine id.
// So any one or two mac's is fine.
quint16 hashMacAddress(PIP_ADAPTER_INFO info)
{
    quint16 hash = 0;
    for ( quint32 i = 0; i < info->AddressLength; i++ )
	{
		hash += ( info->Address[i] << (( i & 1 ) * 8 ));
	}
	return hash;
}

void ThorQ::SystemID_Internal::getMacHash(quint16& mac1, quint16& mac2)
{
	IP_ADAPTER_INFO AdapterInfo[32];
	DWORD dwBufLen = sizeof( AdapterInfo );

	DWORD dwStatus = GetAdaptersInfo( AdapterInfo, &dwBufLen );
	if ( dwStatus != ERROR_SUCCESS )
		return; // no adapters.

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	mac1 = hashMacAddress( pAdapterInfo );
	if ( pAdapterInfo->Next )
		mac2 = hashMacAddress( pAdapterInfo->Next );

	// sort the mac addresses. We don't want to invalidate
	// both macs if they just change order.
	if ( mac1 > mac2 )
	{
        quint16 tmp = mac2;
		mac2 = mac1;
		mac1 = tmp;
	}
}

quint16 ThorQ::SystemID_Internal::getVolumeHash()
{
	DWORD serialNum = 0;

	// Determine if this volume uses an NTFS file system.
	GetVolumeInformation( "c:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0 );
    quint16 hash = (quint16)(( serialNum + ( serialNum >> 16 )) & 0xFFFF );

	return hash;
}

quint16 ThorQ::SystemID_Internal::getCpuHash()
{
	int cpuinfo[4] = { 0, 0, 0, 0 };
	__cpuid( cpuinfo, 0 );
    quint16 hash = 0;
    quint16* ptr = (quint16*)(&cpuinfo[0]);
    for ( quint32 i = 0; i < 8; i++ )
		hash += ptr[i];

	return hash;
}

const char* ThorQ::SystemID_Internal::getMachineName()
{
	static char computerName[1024];
	DWORD size = 1024;
	GetComputerName( computerName, &size );
	return &(computerName[0]);
}
