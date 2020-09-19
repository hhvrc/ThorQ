/// @file thorq_payload_authtoken.h
///
///

#ifndef THORQ_PAYLOAD_AUTHTOKEN_H
#define THORQ_PAYLOAD_AUTHTOKEN_H

#include <vector>
#include <cstdint>

#include <QString>

#include "constants.h"
#include "enums.h"

/// @enum THORQ_PAYLOAD_AUTHTOKEN
enum THORQ_PAYLOAD_AUTHTOKEN
{
    THORQ_PAYLOAD_AUTHTOKEN_DATA,
    THORQ_PAYLOAD_AUTHTOKEN_CMD_REQ,
    THORQ_PAYLOAD_AUTHTOKEN_CMD_AWAITING_INPUT,
    THORQ_PAYLOAD_AUTHTOKEN_CMD_OK
};

/**
 * @brief thorq_payload_authtoken_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_authtoken_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload.size() == 2 && payload[0] == THORQ_PAYLOAD_ID_AUTHTOKEN)
    {
        return payload[1] == THORQ_AUTHTOKEN_CMD_REQ
            || payload[1] == THORQ_AUTHTOKEN_CMD_AWAITING_INPUT
            || payload[1] == THORQ_AUTHTOKEN_CMD_OK;
    }

    return payload[1] == THORQ_AUTHTOKEN_CMD_DATA && payload.size() == THORQ_AUTHTOKEN_LEN + 2;
}

/**
 * @brief thorq_payload_authtoken_pack
 * @param payload
 * @param cmd
 */
inline void thorq_payload_authtoken_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_REGKEY cmd)
{
    payload.resize(2);

    payload[0] = THORQ_PAYLOAD_ID_REGKEY;
    payload[1] = static_cast<std::uint8_t>(cmd);
}

/**
 * @brief thorq_payload_authtoken_pack
 * @param payload
 * @param cmd
 * @param key
 */
inline void thorq_payload_authtoken_pack(std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_REGKEY cmd, const QString& key)
{
    QByteArray keyBytes = key.toUtf8();

    payload.resize(2 + keyBytes.size());

    payload[0] = THORQ_PAYLOAD_ID_REGKEY;
    payload[1] = static_cast<std::uint8_t>(cmd);

    memcpy(payload.data() + 2, keyBytes.data(), keyBytes.size());
}

/**
 * @brief thorq_payload_authtoken_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_authtoken_get_cmd(const std::vector<std::uint8_t>& payload, std::uint8_t& cmd)
{
    cmd = payload[1];
}

/**
 * @brief thorq_payload_authtoken_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_authtoken_get_key(const std::vector<std::uint8_t>& payload, QString& key)
{
    key = QString::fromUtf8((char*)payload.data() + 2, payload.size() - 2);
}

#endif // THORQ_PAYLOAD_AUTHTOKEN_H
