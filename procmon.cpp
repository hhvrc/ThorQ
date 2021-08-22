#include "procmon.h"

#include <iterator>

#ifdef _WIN32
#include <WinSock2.h>
#include <tlhelp32.h>
#elif __linux__
#include <cstdio>
#include <cstring>
#include <cstdlib>
#endif

bool ProcessIsRuning(const char* processName)
{
#ifdef _WIN32
    const char* p = strrchr(processName, '\\');

	if(p)
		processName = p+1;

	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if ( processesSnapshot == INVALID_HANDLE_VALUE )
		return false;

	bool result = false;
    if (Process32First(processesSnapshot, &processInfo))
    {
        do
        {
            result = (strcmp(processName, processInfo.szExeFile) == 0);
        }
        while (!result && Process32Next(processesSnapshot, &processInfo));
	}

	CloseHandle(processesSnapshot);
    return result;
#elif __linux__
    char buf[128];

    snprintf(buf, std::size(buf), "ps | grep %s > /dev/null", processName);

    return system(buf) == 0;
#endif
}
