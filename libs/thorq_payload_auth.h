#ifndef THORQ_PAYLOAD_AUTH_H
#define THORQ_PAYLOAD_AUTH_H

#include "thorq_payload.h"

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
 * @brief thorq_payload_auth_pack
 * @param payload
 * @param state
 * @param data
 */
inline void thorq_payload_auth_pack(thorq_payload_t& payload, const thorq_auth_cmd_t& state, const std::vector<std::uint8_t>& data = std::vector<std::uint8_t>())
{
    payload.id = THORQ_PAYLOAD_ID_AUTH;
	payload.data.resize(1 + data.size());
	payload.data[0] = state;
    if (data.size() != 0)
        memcpy(&payload.data[1], &data[0], data.size());
}

/**
 * @brief thorq_payload_auth_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_auth_is_valid(const thorq_payload_t& payload)
{
    return payload.id == THORQ_PAYLOAD_ID_AUTH;
}

/**
 * @brief thorq_payload_auth_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_auth_get_cmd(const thorq_payload_t& payload, thorq_auth_cmd_t& cmd)
{
    cmd = static_cast<thorq_auth_cmd_t>(payload.data[0]);
}

/**
 * @brief thorq_payload_auth_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_auth_get_data(const thorq_payload_t& payload, std::vector<std::uint8_t>& data)
{
    data.resize(payload.data.size() - 1);
	if (payload.data.size() > 1)
        memcpy(&data[0], &payload.data[1], payload.data.size());
}

#endif // THORQ_PAYLOAD_AUTH_H
