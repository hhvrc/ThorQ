#include "deleteexecutable.h"


#if __linux__
#include <sys/param.h>
#include <unistd.h>
#include <string>
#elif _WIN32
#include <Windows.h>
#endif

void DelMe()
{
#if __linux__
	char path[MAXPATHLEN];
	readlink("/proc/self/exe", path, MAXPATHLEN);
	char cmd[MAXPATHLEN];
    sprintf(cmd, "rm %s", path);
	system(cmd);
#elif _WIN32
    TCHAR szModuleName[MAX_PATH];
    GetModuleFileName(NULL, szModuleName, MAX_PATH);
    MoveFileEx(szModuleName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
#endif
    exit(EXIT_SUCCESS);
}
