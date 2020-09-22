#ifndef THORQ_PAYLOAD_HEARTBEAT_H
#define THORQ_PAYLOAD_HEARTBEAT_H

#include <vector>

#include "enums.h"

/**
 * @brief thorq_payload_heartbeat_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_heartbeat_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload[0] == THORQ_PAYLOAD_ID_HEARTBEAT && payload.size() == 3;
}

/**
 * @brief thorq_payload_heartbeat_pack
 * @param payload
 * @param interval_ms if this is from client it indicates the current interval, if it is from server it sets the client interval
 */
inline void thorq_payload_heartbeat_pack(std::vector<std::uint8_t>& payload, std::uint16_t interval_ms)
{
    payload.resize(3);
    payload[0] = THORQ_PAYLOAD_ID_HEARTBEAT;
    payload[1] = interval_ms >> 8;
    payload[2] = interval_ms >> 0;
}

/**
 * @brief thorq_payload_heartbeat_pack
 * @param payload
 */
inline void thorq_payload_heartbeat_unpack(const std::vector<std::uint8_t>& payload, std::uint16_t& interval_ms)
{
    interval_ms = payload[1] << 8;
    interval_ms = payload[2] << 0;
}

#endif // THORQ_PAYLOAD_HEARTBEAT_H
