/// @file thorq_payload_auth.h
///
///

#ifndef THORQ_PAYLOAD_AUTH_H
#define THORQ_PAYLOAD_AUTH_H

#include <vector>

#include <QString>
#include <QByteArray>

#include "constants.h"
#include "enums.h"

/// @enum THORQ_PAYLOAD_AUTH
enum THORQ_PAYLOAD_AUTH : std::uint8_t
{
    THORQ_PAYLOAD_AUTH_SYSTEMID,
    THORQ_PAYLOAD_AUTH_SYSTEMID_REQ,

    THORQ_PAYLOAD_AUTH_REGKEY,
    THORQ_PAYLOAD_AUTH_REGKEY_REQ,
    THORQ_PAYLOAD_AUTH_REGKEY_AWAITING_INPUT,

    THORQ_PAYLOAD_AUTH_OK,
};

/**
 * @brief thorq_payload_auth_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_auth_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_AUTH) return false;

    if (payload.size() == 2)
    {
        return payload[1] == THORQ_PAYLOAD_AUTH_SYSTEMID_REQ
            || payload[1] == THORQ_PAYLOAD_AUTH_REGKEY_REQ
            || payload[1] == THORQ_PAYLOAD_AUTH_REGKEY_AWAITING_INPUT
            || payload[1] == THORQ_PAYLOAD_AUTH_OK;
    }

    return (payload[1] == THORQ_PAYLOAD_AUTH_REGKEY
            && payload.size() == THORQ_AUTH_REGKEY_LEN)
        || (payload[1] == THORQ_PAYLOAD_AUTH_SYSTEMID
            && payload.size() >= THORQ_AUTH_SYSTEMID_LEN_MIN + 2
            && payload.size() <= THORQ_AUTH_SYSTEMID_LEN_MAX + 2);
}

/**
 * @brief thorq_payload_auth_pack
 * @param payload
 * @param cmd
 */
inline void thorq_payload_auth_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_AUTH cmd)
{
	payload.resize(2);

    payload[0] = THORQ_PAYLOAD_ID_AUTH;
    payload[1] = static_cast<std::uint8_t>(cmd);
}

/**
 * @brief thorq_payload_auth_pack
 * @param payload
 * @param cmd
 * @param data
 */
inline void thorq_payload_auth_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_AUTH cmd, const QByteArray& data)
{
	payload.resize(2 + data.size());

    payload[0] = THORQ_PAYLOAD_ID_AUTH;
	payload[1] = cmd;

	memcpy(payload.data() + 2, data.data(), data.size());
}

/**
 * @brief thorq_payload_auth_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_auth_get_cmd(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_AUTH& cmd)
{
    cmd = static_cast<THORQ_PAYLOAD_AUTH>(payload[1]);
}

/**
 * @brief thorq_payload_auth_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_auth_get_data(const std::vector<std::uint8_t>& payload, QByteArray& data)
{
    data.resize((int)payload.size() - 2);

	memcpy(data.data(), payload.data() + 2, payload.size() - 2);
}

#endif // THORQ_PAYLOAD_AUTH_H
