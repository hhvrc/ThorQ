#include "deleteexecutable.h"

#include <fmt/core.h>

#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
#include <WinSock2.h>
#elif __linux__
#include <sys/param.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#endif

void DelMe()
{
#if __linux__
    char cmd[std::size("rm ") + MAXPATHLEN];

	strcpy(&cmd[0], "rm ");

	if (readlink("/proc/self/exe", &cmd[3], MAXPATHLEN) == -1)
	{
        char arr[256]{0};
        strerror_r(errno, arr, 256);

        fmt::print(stderr, "Error getting path to self: {}\n", arr);
	}

    fmt::print("System call success: {}\n", system(cmd) == EXIT_SUCCESS);
#elif _WIN32
    TCHAR szModuleName[MAX_PATH];
    GetModuleFileName(NULL, szModuleName, MAX_PATH);
    MoveFileEx(szModuleName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
#endif
    exit(EXIT_SUCCESS);
}
