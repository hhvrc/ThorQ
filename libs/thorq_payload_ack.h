#ifndef THORQ_PAYLOAD_ACK_H
#define THORQ_PAYLOAD_ACK_H

#include <vector>
#include <cstdint>
#include <QString>

#include "enums.h"

/**
 * @brief thorq_payload_ack_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_ack_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() == 4
        && payload[0] == THORQ_PAYLOAD_ID_ACK
        && payload[1] <  THORQ_PAYLOAD_ID_ACK // payload id that gets acked can be anything else than an ack
        && payload[3] <= THORQ_PAYLOAD_ACK_UNAUTHORIZED; // Enum max
}

/**
 * @brief thorq_payload_ack_pack
 * @param payload
 * @param id
 * @param cmd
 */
inline void thorq_payload_ack_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ID id, std::uint8_t cmd, THORQ_PAYLOAD_ACK ack)
{
    payload.resize(4);
	payload[0] = THORQ_PAYLOAD_ID_ACK;
	payload[1] = id;
	payload[2] = cmd;
    payload[3] = ack;
}

/**
 * @brief thorq_payload_ack_get_id
 * @param payload
 * @param id
 */
inline void thorq_payload_ack_get_id(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ID& id)
{
	id = static_cast<THORQ_PAYLOAD_ID>(payload[1]);
}

/**
 * @brief thorq_payload_ack_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_ack_get_cmd(const std::vector<std::uint8_t>& payload, std::uint8_t& cmd)
{
	cmd = payload[2];
}

/**
 * @brief thorq_payload_ack_get_ack
 * @param payload
 * @param cmd
 */
inline void thorq_payload_ack_get_ack(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ACK& ack)
{
    ack = static_cast<THORQ_PAYLOAD_ACK>(payload[3]);
}

#endif // THORQ_PAYLOAD_ACK_H
