#ifndef THORQ_PAYLOAD_COLLAR_H
#define THORQ_PAYLOAD_COLLAR_H

#include <vector>

#include "enums.h"

/**
 * @brief thorq_payload_collar_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_collar_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() == 6 && payload[0] == THORQ_PAYLOAD_ID_COLLAR;
}

/**
 * @brief thorq_payload_collar_pack
 * @param payload
 * @param flags
 * @param shock_value
 * @param vibration_value
 * @param beep_value
 * @param auto_value
 */
inline void thorq_payload_collar_pack(std::vector<std::uint8_t>& payload, std::uint8_t flags, std::uint8_t shock_value, std::uint8_t vibration_value, std::uint8_t beep_value, std::uint8_t auto_value)
{
    payload.resize(6);
    payload[0] = THORQ_PAYLOAD_ID_COLLAR;
    payload[1] = flags;
    payload[2] = shock_value;
    payload[3] = vibration_value;
    payload[4] = beep_value;
    payload[5] = auto_value;
}

/**
 * @brief thorq_payload_collar_unpack
 * @param payload
 * @param flags
 * @param shock_value
 * @param vibration_value
 * @param beep_value
 * @param auto_value
 */
inline void thorq_payload_collar_unpack(const std::vector<std::uint8_t>& payload, std::uint8_t& flags, std::uint8_t& shock_value, std::uint8_t& vibration_value, std::uint8_t& beep_value, std::uint8_t& auto_value)
{
    flags           = payload[1];
    shock_value     = payload[2];
    vibration_value = payload[3];
    beep_value      = payload[4];
    auto_value      = payload[5];
}

#endif // THORQ_PAYLOAD_COLLAR_H
