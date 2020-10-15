#ifndef VERSION_H
#define VERSION_H

#include <cstdint>
#include <string>

namespace ThorQ {
/// Version type
struct Version
{
    std::uint8_t major;
    std::uint8_t minor;
    std::uint8_t patch;

    std::string toString() const;

    inline Version operator- (const Version& other)
    {
        Version diff;
        diff.major = major - other.major;
        diff.minor = minor - other.minor;
        diff.patch = patch - other.patch;
        return diff;
    }

    inline Version &operator-=(const Version& other)
    {
        *this = *this - other;
        return *this;
    }

    inline bool operator==(const Version& other) const { return this->major == other.major && this->minor == other.minor && this->patch == other.patch; }
    inline bool operator!=(const Version& other) const { return !(*this == other); }
    inline bool operator< (const Version& other) const { return this->major <  other.major || this->minor <  other.minor || this->patch <  other.patch; }
    inline bool operator<=(const Version& other) const { return !(other < *this); }
    inline bool operator> (const Version& other) const { return other < *this; }
    inline bool operator>=(const Version& other) const { return !(*this < other); }
};
}

#endif // VERSION_H
