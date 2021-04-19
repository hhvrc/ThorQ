#ifndef VERSION_H
#define VERSION_H

#include "schemas/version_generated.h"

#include <string>
#include <cstdint>

namespace ThorQ {
class Version
{
public:
    constexpr Version() noexcept
        : m_major(0)
        , m_minor(0)
        , m_patch(0)
    {}

    constexpr Version(std::uint8_t major, std::uint8_t minor, std::uint8_t patch) noexcept
        : m_major(major)
        , m_minor(minor)
        , m_patch(patch)
    {}

    constexpr Version(const ThorQ::Version& other) noexcept
        : m_major(other.m_major)
        , m_minor(other.m_minor)
        , m_patch(other.m_patch)
    {}

    Version(const ThorQ::Serialization::Version* other) noexcept
        : m_major(other->major())
        , m_minor(other->minor())
        , m_patch(other->patch())
    {}

    constexpr std::uint8_t major() const noexcept { return m_major; }
    constexpr std::uint8_t minor() const noexcept { return m_minor; }
    constexpr std::uint8_t patch() const noexcept { return m_patch; }

    constexpr void setMajor(std::uint8_t major) noexcept { m_major = major; }
    constexpr void setMinor(std::uint8_t minor) noexcept { m_minor = minor; }
    constexpr void setPatch(std::uint8_t patch) noexcept { m_patch = patch; }

    std::string toString() const;
private:
    std::uint8_t m_major;
    std::uint8_t m_minor;
    std::uint8_t m_patch;
};

constexpr bool operator == (const ThorQ::Version& lhs, const ThorQ::Version& rhs) noexcept { return lhs.major() == rhs.major() && lhs.minor() == rhs.minor() && lhs.patch() == rhs.patch(); }
constexpr bool operator <  (const ThorQ::Version& lhs, const ThorQ::Version& rhs) noexcept { return lhs.major() <  rhs.major() || lhs.minor() <  rhs.minor() || lhs.patch() <  rhs.patch(); }
constexpr bool operator >  (const ThorQ::Version& lhs, const ThorQ::Version& rhs) noexcept { return lhs.major() >  rhs.major() || lhs.minor() >  rhs.minor() || lhs.patch() >  rhs.patch(); }
constexpr bool operator != (const ThorQ::Version& lhs, const ThorQ::Version& rhs) noexcept { return !(lhs == rhs); }
constexpr bool operator <= (const ThorQ::Version& lhs, const ThorQ::Version& rhs) noexcept { return !(lhs >  rhs); }
constexpr bool operator >= (const ThorQ::Version& lhs, const ThorQ::Version& rhs) noexcept { return !(lhs <  rhs); }

inline bool operator == (const ThorQ::Version& lhs, const ThorQ::Serialization::Version* rhs) { return lhs.major() == rhs->major() && lhs.minor() == rhs->minor() && lhs.patch() == rhs->patch(); }
inline bool operator <  (const ThorQ::Version& lhs, const ThorQ::Serialization::Version* rhs) { return lhs.major() <  rhs->major() || lhs.minor() <  rhs->minor() || lhs.patch() <  rhs->patch(); }
inline bool operator >  (const ThorQ::Version& lhs, const ThorQ::Serialization::Version* rhs) { return lhs.major() >  rhs->major() || lhs.minor() >  rhs->minor() || lhs.patch() >  rhs->patch(); }
inline bool operator != (const ThorQ::Version& lhs, const ThorQ::Serialization::Version* rhs) { return !(lhs == rhs); }
inline bool operator <= (const ThorQ::Version& lhs, const ThorQ::Serialization::Version* rhs) { return !(lhs >  rhs); }
inline bool operator >= (const ThorQ::Version& lhs, const ThorQ::Serialization::Version* rhs) { return !(lhs <  rhs); }

inline bool operator == (const ThorQ::Serialization::Version* lhs, const ThorQ::Version& rhs) { return lhs->major() == rhs.major() && lhs->minor() == rhs.minor() && lhs->patch() == rhs.patch(); }
inline bool operator <  (const ThorQ::Serialization::Version* lhs, const ThorQ::Version& rhs) { return lhs->major() <  rhs.major() || lhs->minor() <  rhs.minor() || lhs->patch() <  rhs.patch(); }
inline bool operator >  (const ThorQ::Serialization::Version* lhs, const ThorQ::Version& rhs) { return lhs->major() >  rhs.major() || lhs->minor() >  rhs.minor() || lhs->patch() >  rhs.patch(); }
inline bool operator != (const ThorQ::Serialization::Version* lhs, const ThorQ::Version& rhs) { return !(lhs == rhs); }
inline bool operator <= (const ThorQ::Serialization::Version* lhs, const ThorQ::Version& rhs) { return !(lhs >  rhs); }
inline bool operator >= (const ThorQ::Serialization::Version* lhs, const ThorQ::Version& rhs) { return !(lhs <  rhs); }
}

#endif // VERSION_H
