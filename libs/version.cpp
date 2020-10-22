#include "version.h"
#include "fmt/core.h"

std::string ThorQ::Version::toString() const
{
    return fmt::format("{}.{}.{}", major, minor, patch);
}
