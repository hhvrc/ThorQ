#include "deleteexecutable.h"


#if __linux__
#include <sys/param.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <cstdlib>
#elif _WIN32
#include <Windows.h>
#endif

void DelMe()
{
#if __linux__
	// TODO: unused
	char path[MAXPATHLEN];
	if (readlink("/proc/self/exe", path, MAXPATHLEN) == -1)
	{
		fprintf(stderr, "Error getting path to self: %s\n", strerror(errno));
	}
	char cmd[MAXPATHLEN];
    sprintf(cmd, "rm %s", path);
	printf("System call %s\n", system(cmd) == EXIT_SUCCESS ? "succeeded" : "failed");
#elif _WIN32
    TCHAR szModuleName[MAX_PATH];
    GetModuleFileName(NULL, szModuleName, MAX_PATH);
    MoveFileEx(szModuleName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
#endif
    exit(EXIT_SUCCESS);
}
