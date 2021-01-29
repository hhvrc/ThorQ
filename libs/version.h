#ifndef VERSION_H
#define VERSION_H

#include "schemas/version_generated.h"

#include <string>
#include <cstdint>

namespace ThorQ {
/// Version type
class Version
{
public:
    Version();
    Version(std::uint8_t major, std::uint8_t minor, std::uint8_t patch);
    Version(const ThorQ::Version& version);
    Version(const ThorQ::Serialization::Version* version);

    std::uint8_t major() const;
    std::uint8_t minor() const;
    std::uint8_t patch() const;

    void setMajor(std::uint8_t major);
    void setMinor(std::uint8_t minor);
    void setPatch(std::uint8_t patch);

    std::string toString() const;
private:
    std::uint8_t m_major;
    std::uint8_t m_minor;
    std::uint8_t m_patch;
};

inline bool operator == (const ThorQ::Version& lhs, const ThorQ::Version& rhs) { return lhs.major() == rhs.major() && lhs.minor() == rhs.minor() && lhs.patch() == rhs.patch(); }
inline bool operator <  (const ThorQ::Version& lhs, const ThorQ::Version& rhs) { return lhs.major() <  rhs.major() || lhs.minor() <  rhs.minor() || lhs.patch() <  rhs.patch(); }
inline bool operator >  (const ThorQ::Version& lhs, const ThorQ::Version& rhs) { return lhs.major() >  rhs.major() || lhs.minor() >  rhs.minor() || lhs.patch() >  rhs.patch(); }
inline bool operator != (const ThorQ::Version& lhs, const ThorQ::Version& rhs) { return !(lhs == rhs); }
inline bool operator <= (const ThorQ::Version& lhs, const ThorQ::Version& rhs) { return !(lhs >  rhs); }
inline bool operator >= (const ThorQ::Version& lhs, const ThorQ::Version& rhs) { return !(lhs <  rhs); }

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
