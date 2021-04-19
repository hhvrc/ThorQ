#include "version.h"

#include "fmt/compile.h"
#include "fmt/core.h"

std::string ThorQ::Version::toString() const
{
    return fmt::format(FMT_COMPILE("{}.{}.{}"), m_major, m_minor, m_patch);
}
