#ifndef THORQ_PAYLOAD_VERSION_H
#define THORQ_PAYLOAD_VERSION_H

#include <string>

#include "enums.h"
#include "constants.h"
#include "thorq_payload.h"

/** @file thorq_payload_version.h
 *
 */

/** @typedef thorq_version_t
 *
 */
typedef struct __thorq_version {
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
		memset(buffer, 0, 12);
		int cx = snprintf(buffer, 12, "%u.%u.%u", major, minor, patch);

		return std::string(buffer, cx);
	}

    /**
     * @brief operator-
     * @param other
     * @return
     */
	__thorq_version operator- (const __thorq_version& other)
	{
		__thorq_version diff;
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
    __thorq_version& operator-=(const __thorq_version& other)
    {
        return *this; *this = *this - other;
    };

	bool operator==(const __thorq_version& other) { return major == other.major && minor == other.minor && patch == other.patch; }
	bool operator!=(const __thorq_version& other) { return major != other.major || minor != other.minor || patch != other.patch; }
	bool operator< (const __thorq_version& other) { return major <  other.major || minor <  other.minor || patch <  other.patch; }
	bool operator> (const __thorq_version& other) { return major >  other.major || minor >  other.minor || patch >  other.patch; }
	bool operator<=(const __thorq_version& other) { return *this < other || *this == other; }
	bool operator>=(const __thorq_version& other) { return *this > other || *this == other; }
} thorq_version_t;

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
 * @brief thorq_payload_version_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_version_is_valid(const thorq_payload_t& payload)
{
	return payload.id == THORQ_PAYLOAD_ID_VERSION && payload.data.size() == 4;
}

/**
 * @brief thorq_payload_version_pack
 * @param payload
 * @param app
 * @param version
 */
inline void thorq_payload_version_pack(thorq_payload_t& payload, const std::uint8_t& app, const thorq_version_t& version)
{
	payload.id = THORQ_PAYLOAD_ID_VERSION;
	payload.data.resize(4);
	payload.data[0] = app;
	payload.data[1] = version.major;
	payload.data[2] = version.minor;
	payload.data[3] = version.patch;
}

/**
 * @brief thorq_payload_version_unpack
 * @param payload
 * @param app
 * @param version
 */
inline void thorq_payload_version_unpack(const thorq_payload_t& payload, std::uint8_t& app, thorq_version_t& version)
{
	app = payload.data[0];
	version.major = payload.data[1];
	version.minor = payload.data[2];
	version.patch = payload.data[3];
}

#endif // THORQ_PAYLOAD_VERSION_H
