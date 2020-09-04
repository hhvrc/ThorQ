#ifndef THORQ_PAYLOAD_VERSION_H
#define THORQ_PAYLOAD_VERSION_H
/** @file thorq_message_version.h
 *
 */

#include <vector>
#include <string>
#include <cstring>
#include <cstdint>

#include "enums.h"
#include "constants.h"


/** @typedef thorq_version_t
 *
 */
struct thorq_version_t
{
	std::uint8_t major;
	std::uint8_t minor;
	std::uint8_t patch;

    /**
     * @brief to_string
     * @return
     */
	inline std::string to_string() const
	{
        char buffer[12];
		int cx = snprintf(buffer, 12, "%u.%u.%u", major, minor, patch);
		buffer[cx] = 0;

		return std::string(buffer, cx);
	}

    /**
     * @brief operator-
     * @param other
     * @return
     */
    thorq_version_t operator- (const thorq_version_t& other)
	{
        thorq_version_t diff;
		diff.major = major - other.major;
		diff.minor = minor - other.minor;
		diff.patch = patch - other.patch;
		return diff;
	};

    /**
     * @brief operator-=
     * @param other
     * @return
     */
    thorq_version_t& operator-=(const thorq_version_t& other)
    {
        return *this; *this = *this - other;
    };

	bool operator==(const thorq_version_t& other) const { return this->major == other.major && this->minor == other.minor && this->patch == other.patch; }
	bool operator!=(const thorq_version_t& other) const { return !(*this == other); }
	bool operator< (const thorq_version_t& other) const { return this->major <  other.major || this->minor <  other.minor || this->patch <  other.patch; }
	bool operator<=(const thorq_version_t& other) const { return !(other < *this); }
	bool operator> (const thorq_version_t& other) const { return other < *this; }
	bool operator>=(const thorq_version_t& other) const { return !(*this < other); }
};

/** @value THORQ_VERSION_SERVER
 *
 */
constexpr thorq_version_t THORQ_VERSION_SERVER { THORQ_VERSION_SERVER_MAJOR, THORQ_VERSION_SERVER_MINOR, THORQ_VERSION_SERVER_PATCH };

/** @value THORQ_VERSION_CLIENT
 *
 */
constexpr thorq_version_t THORQ_VERSION_CLIENT { THORQ_VERSION_CLIENT_MAJOR, THORQ_VERSION_CLIENT_MINOR, THORQ_VERSION_CLIENT_PATCH };

/** @value THORQ_VERSION_LINK
 *
 */
constexpr thorq_version_t THORQ_VERSION_LINK   { THORQ_VERSION_LINK_MAJOR,   THORQ_VERSION_LINK_MINOR,   THORQ_VERSION_LINK_PATCH   };

/**
 * @brief thorq_message_version_is_valid
 * @param payload
 * @return
 */
inline bool thorq_message_version_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() == 5 && payload[0] == THORQ_MESSAGE_ID_VERSION;
}

/**
 * @brief thorq_message_version_pack
 * @param payload
 * @param app
 * @param version
 */
inline void thorq_message_version_pack(std::vector<std::uint8_t>& payload, THORQ_APP app, const thorq_version_t& version)
{
    payload.resize(5);
    payload[0] = THORQ_MESSAGE_ID_VERSION;
    payload[1] = app;
    payload[2] = version.major;
    payload[3] = version.minor;
    payload[4] = version.patch;
}

/**
 * @brief thorq_message_version_unpack
 * @param payload
 * @param app
 * @param version
 */
inline void thorq_message_version_unpack(const std::vector<std::uint8_t>& payload, THORQ_APP& app, thorq_version_t& version)
{
    app = payload[1];
    version.major = payload[2];
    version.minor = payload[3];
    version.patch = payload[4];
}

#endif // THORQ_PAYLOAD_VERSION_H
