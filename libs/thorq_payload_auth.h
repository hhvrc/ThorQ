#ifndef THORQ_PAYLOAD_AUTH_H
#define THORQ_PAYLOAD_AUTH_H

#include <array>
#include <vector>

#include "constants.h"
#include "enums.h"

/** @typedef thorq_auth_cmd_t
 *
 */
typedef enum {
	THORQ_AUTH_SYSTEMID_REQ,
	THORQ_AUTH_SYSTEMID,

	THORQ_AUTH_REGKEY_REQ,
	THORQ_AUTH_REGKEY_AWAITING_INPUT,
	THORQ_AUTH_REGKEY,

	THORQ_AUTH_OK,
} thorq_auth_cmd_t;

/**
 * @brief thorq_message_auth_pack
 * @param payload
 * @param state
 * @param data
 */
inline void thorq_message_auth_pack(std::vector<std::uint8_t>& payload, const thorq_auth_cmd_t& cmd, const std::vector<std::uint8_t>& data = std::vector<std::uint8_t>())
{
    payload.resize(2 + data.size());

    payload[0] = THORQ_MESSAGE_ID_AUTH;
    payload[1] = cmd;

    memcpy(payload.data() + 2, data.data(), data.size());
}

/**
 * @brief thorq_message_auth_is_valid
 * @param payload
 * @return
 */
inline bool thorq_message_auth_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() >= 2 && payload[0] == THORQ_MESSAGE_ID_AUTH;
}

/**
 * @brief thorq_message_auth_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_message_auth_get_cmd(const std::vector<std::uint8_t>& payload, thorq_auth_cmd_t& cmd)
{
    cmd = static_cast<thorq_auth_cmd_t>(payload[1]);
}

/**
 * @brief thorq_message_auth_get_data
 * @param payload
 * @param data
 */
inline void thorq_message_auth_get_data(const std::vector<std::uint8_t>& payload, std::vector<std::uint8_t>& data)
{
    data.resize(payload.size() - 2);

    memcpy(data.data(), payload.data() + 2, payload.size() - 2);
}

/**
 * @brief thorq_message_auth_get_data
 * @param payload
 * @param data
 */
inline void thorq_message_auth_get_data(const std::vector<std::uint8_t>& payload, std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN>& data)
{
    memcpy(data.data(), payload.data() + 2, std::min(payload.size() - 2, THORQ_AUTH_REGKEY_LEN));
}

#endif // THORQ_PAYLOAD_AUTH_H
