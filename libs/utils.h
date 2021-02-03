#ifndef UTILS_FILESYSTEM_H
#define UTILS_FILESYSTEM_H

#include <vector>
#include <string>
#include <string.h>
#include <cstdint>

#define THORQ_UNUSED(x) (void)x;

bool tryWriteAll(const std::string& path, const std::vector<std::uint8_t>& data);
bool tryReadAll(const std::string& path, std::vector<std::uint8_t>& data, std::size_t sizeMax = SIZE_MAX);
inline std::string errno_str(int err)
{
    char buf[512];
#ifdef _WIN32
    strerror_s(buf, sizeof(buf), err);
    return buf;
#else
    return strerror_r(err, buf, sizeof(buf));
#endif
}

#endif // UTILS_FILESYSTEM_H
