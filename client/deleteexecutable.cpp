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
    char buf[std::size("rm ") + MAXPATHLEN]{0};

    memcpy(buf, "rm ", 3);

    if (readlink("/proc/self/exe", &buf[3], MAXPATHLEN) == -1)
    {
        fmt::print(stderr, "Error getting path to self: {}\n", strerror_r(errno, buf, 256));
	}

    fmt::print("System call success: {}\n", system(buf) == EXIT_SUCCESS);
#elif _WIN32
    TCHAR szModuleName[MAX_PATH];
    GetModuleFileName(NULL, szModuleName, MAX_PATH);
    MoveFileEx(szModuleName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
#endif
    exit(EXIT_SUCCESS);
}
