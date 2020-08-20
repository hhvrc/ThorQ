#ifndef THORQ_PAYLOAD_HEARTBEAT_H
#define THORQ_PAYLOAD_HEARTBEAT_H

#include <vector>

#include "enums.h"

/**
 * @brief thorq_message_heartbeat_is_valid
 * @param payload
 * @return
 */
inline bool thorq_message_heartbeat_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() == 1 && payload[0] == THORQ_MESSAGE_ID_HEARTBEAT;
}

/**
 * @brief thorq_message_heartbeat_pack
 * @param payload
 */
inline void thorq_message_heartbeat_pack(std::vector<std::uint8_t>& payload)
{
    payload.resize(1);
    payload[0] = THORQ_MESSAGE_ID_HEARTBEAT;
}

#endif // THORQ_PAYLOAD_HEARTBEAT_H
