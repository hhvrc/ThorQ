#include "deleteexecutable.h"


#if __linux__
#include <sys/param.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#elif _WIN32
#include <Windows.h>
#endif

void DelMe()
{
#if __linux__
	char cmd[sizeof("rm ") + MAXPATHLEN];

	strcpy(&cmd[0], "rm ");

	if (readlink("/proc/self/exe", &cmd[3], MAXPATHLEN) == -1)
	{
		fprintf(stderr, "Error getting path to self: %s\n", strerror(errno));
	}

	printf("System call %s\n", system(cmd) == EXIT_SUCCESS ? "succeeded" : "failed");
#elif _WIN32
    TCHAR szModuleName[MAX_PATH];
    GetModuleFileName(NULL, szModuleName, MAX_PATH);
    MoveFileEx(szModuleName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
#endif
    exit(EXIT_SUCCESS);
}
