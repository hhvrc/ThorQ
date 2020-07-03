#include "procmon.h"

#if __linux__
#include <cstdio>
#include <cstring>
#include <cstdlib>
bool ProcessIsRuning(const char* processName)
{
    char buf[128]{0};

    snprintf(buf, 128, "ps | grep %s > /dev/null", processName);

    return system(buf) == 0;
}
#elif _WIN32
//#include <tlhelp32.h>
bool ProcessIsRuning(const char* processName)
{
    /*
    char* p = strrchr(processName, '\\');
    if(p)
        processName = p+1;

    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if ( processesSnapshot == INVALID_HANDLE_VALUE )
        return 0;

    Process32First(processesSnapshot, &processInfo);
    if ( !strcmp(processName, processInfo.szExeFile) )
    {
        CloseHandle(processesSnapshot);
        return processInfo.th32ProcessID;
    }

    while ( Process32Next(processesSnapshot, &processInfo) )
    {
        if ( !strcmp(processName, processInfo.szExeFile) )
        {
          CloseHandle(processesSnapshot);
          return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processesSnapshot);
    */
    return 0;
}
#endif
