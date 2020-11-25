#include "version.h"
#include "fmt/core.h"

ThorQ::Version::Version()
    : m_major(0)
    , m_minor(0)
    , m_patch(0)
{}

ThorQ::Version::Version(std::uint8_t major, std::uint8_t minor, std::uint8_t patch)
    : m_major(major)
    , m_minor(minor)
    , m_patch(patch)
{}

ThorQ::Version::Version(const ThorQ::Version& other)
    : m_major(other.major())
    , m_minor(other.minor())
    , m_patch(other.patch())
{}

ThorQ::Version::Version(const ThorQ::Serialization::Version* other)
    : m_major(other->major())
    , m_minor(other->minor())
    , m_patch(other->patch())
{}

std::uint8_t ThorQ::Version::major() const
{
    return m_major;
}
std::uint8_t ThorQ::Version::minor() const
{
    return m_minor;
}
std::uint8_t ThorQ::Version::patch() const
{
    return m_patch;
}

void ThorQ::Version::setMajor(uint8_t major)
{
    m_major = major;
}

void ThorQ::Version::setMinor(uint8_t minor)
{
    m_minor = minor;
}

void ThorQ::Version::setPatch(uint8_t patch)
{
    m_patch = patch;
}

std::string ThorQ::Version::toString() const
{
    return fmt::format("{}.{}.{}", m_major, m_minor, m_patch);
}
