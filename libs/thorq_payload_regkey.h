/// @file thorq_payload_regkey.h
///
///

#ifndef THORQ_PAYLOAD_REGKEY_H
#define THORQ_PAYLOAD_REGKEY_H

#include <vector>
#include <cstdint>

#include <QString>

#include "constants.h"
#include "enums.h"

/// @enum THORQ_PAYLOAD_REGKEY
enum THORQ_PAYLOAD_REGKEY : std::uint8_t
{
    THORQ_PAYLOAD_REGKEY_REQ,
    THORQ_PAYLOAD_REGKEY_AWAITING_INPUT,
    THORQ_PAYLOAD_REGKEY_DATA,
    THORQ_PAYLOAD_REGKEY_OK
};

inline bool thorq_payload_regkey_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_REGKEY) return false;

    if (payload.size() >= 2)
    {
        switch (payload[1]) {
        case THORQ_PAYLOAD_REGKEY_REQ:
        case THORQ_PAYLOAD_REGKEY_AWAITING_INPUT:
        case THORQ_PAYLOAD_REGKEY_OK:
            return payload.size() == 2;
        case THORQ_PAYLOAD_REGKEY_DATA:
            return payload.size() == THORQ_AUTH_REGKEY_LEN + 2;
        default:
            return false;
        }
    }

    return false;
}

inline void thorq_payload_regkey_pack(std::vector<std::uint8_t>& payload, THORQ_REGKEY_CMD cmd)
{
    payload.resize(2);

    payload[0] = THORQ_PAYLOAD_ID_REGKEY;
    payload[1] = static_cast<std::uint8_t>(cmd);
}

/**
 * @brief thorq_payload_regkey_pack
 * @param payload
 * @param cmd
 * @param key
 */
inline void thorq_payload_regkey_pack(std::vector<std::uint8_t>& payload, THORQ_REGKEY_CMD cmd, const QString& key)
{
    QByteArray keyBytes = key.toUtf8();

    payload.resize(2 + keyBytes.size());

    payload[0] = THORQ_PAYLOAD_ID_REGKEY;
    payload[1] = static_cast<std::uint8_t>(cmd);

    memcpy(payload.data() + 2, keyBytes.data(), keyBytes.size());
}

/**
 * @brief thorq_payload_regkey_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_regkey_get_cmd(const std::vector<std::uint8_t>& payload, THORQ_REGKEY_CMD& cmd)
{
    cmd = payload[1];
}

/**
 * @brief thorq_payload_regkey_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_regkey_get_key(const std::vector<std::uint8_t>& payload, QString& key)
{
    key = QString::fromUtf8((char*)payload.data() + 2, payload.size() - 2);
}

#endif // THORQ_PAYLOAD_REGKEY_H
