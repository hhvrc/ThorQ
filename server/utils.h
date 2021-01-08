#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <string.h>

std::string errno_str(int err)
{
    std::string str;
    str.resize(512);
#ifdef _WIN32
    strerror_s(str.data(), str.size(), err);
#else
    strerror_r(err, str.data(), str.size());
#endif
    str.resize(strlen(str.data()));
    return str;
}

#endif // UTILS_H
