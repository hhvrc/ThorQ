#include "deleteexecutable.h"


#if __linux__
#include <string>
#include <libgen.h>
#elif _WIN32
#include <Windows.h>
#endif

void DelMe()
{
#if __linux__
    char* path[PATH_MAX];
    readlink("/proc/self/exe", path, PATH_MAX);
    char* cmd[PATH_MAX];
    sprintf(cmd, "rm %s", path);
    system(cmd);
}
#elif _WIN32
    TCHAR szModuleName[MAX_PATH];
    GetModuleFileName(NULL, szModuleName, MAX_PATH);
    MoveFileEx(szModuleName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
#endif
    exit(EXIT_SUCCESS);
}
