#include "procmon.h"

#if __linux__
#include <cstdio>
#include <cstring>
#include <cstdlib>
#elif _WIN32
//#include <tlhelp32.h>
#endif

bool ProcessIsRuning(const char* processName)
{
#if __linux__
	char buf[128]{0};

	snprintf(buf, 128, "ps | grep %s > /dev/null", processName);

	return system(buf) == 0;
#elif _WIN32
	/*
	char* p = strrchr(processName, '\\');

	if(p)
		processName = p+1;

	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if ( processesSnapshot == INVALID_HANDLE_VALUE )
		return false;

	bool result = false;
	if (Process32First(processesSnapshot, &processInfo)) {
		do { result = (strcmp(processName, processInfo.szExeFile) == 0); }
		while (!result && Process32Next(processesSnapshot, &processInfo))
	}

	CloseHandle(processesSnapshot);
	return result;
	*/
	return true;
#endif
}
