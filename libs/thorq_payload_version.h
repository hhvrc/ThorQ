/// @file thorq_payload_version.h
///
///

#ifndef THORQ_PAYLOAD_VERSION_H
#define THORQ_PAYLOAD_VERSION_H

#include <vector>
#include <cstdint>

#include "enums.h"
#include "version.h"

/**
 * @brief thorq_payload_version_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_version_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() == 5
        && payload[0] == THORQ_PAYLOAD_ID_VERSION
        && payload[1] <= THORQ_APP_LINK; // Max of enum
}

/**
 * @brief thorq_payload_version_pack
 * @param payload
 * @param app
 * @param version
 */
inline void thorq_payload_version_pack(std::vector<std::uint8_t>& payload, THORQ_APP app, ThorQ::Version version)
{
    payload.resize(5);
    payload[0] = THORQ_PAYLOAD_ID_VERSION;
	payload[1] = static_cast<std::uint8_t>(app);
    payload[2] = version.major;
    payload[3] = version.minor;
    payload[4] = version.patch;
}

/**
 * @brief thorq_payload_version_unpack
 * @param payload
 * @param app
 * @param version
 */
inline void thorq_payload_version_unpack(const std::vector<std::uint8_t>& payload, THORQ_APP& app, ThorQ::Version& version)
{
    app = static_cast<THORQ_APP>(payload[1]);
    version.major = payload[2];
    version.minor = payload[3];
    version.patch = payload[4];
}

#endif // THORQ_PAYLOAD_VERSION_H
