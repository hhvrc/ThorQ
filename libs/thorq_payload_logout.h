#ifndef THORQ_PAYLOAD_LOGOUT_H
#define THORQ_PAYLOAD_LOGOUT_H

#include <vector>

#include "enums.h"

/**
 * @brief thorq_payload_logout_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_logout_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload[0] == THORQ_PAYLOAD_ID_LOGOUT;
}

/**
 * @brief thorq_payload_logout_pack
 * @param payload
 */
inline void thorq_payload_logout_pack(std::vector<std::uint8_t>& payload)
{
    payload.resize(1);
    payload[0] = THORQ_PAYLOAD_ID_LOGOUT;
}

#endif // THORQ_PAYLOAD_LOGOUT_H
