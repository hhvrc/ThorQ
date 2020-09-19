#include "version.h"

QString ThorQ::Version::toString() const
{
    char buffer[12];
    int cx = snprintf(buffer, 12, "%u.%u.%u", major, minor, patch);
    buffer[cx] = 0;

    return buffer;
}
