#ifndef THORQ_PAYLOAD_CRYPTO_H
#define THORQ_PAYLOAD_CRYPTO_H

#include <vector>
#include <QByteArray>

#include "enums.h"

/** @file thorq_payload_crypto.h
 *
 */

/** @typedef thorq_crypto_cmd_t
 *
 */
enum thorq_crypto_cmd_t
{
    THORQ_CRYPTO_REQUEST,
    THORQ_CRYPTO_ESTABLISH,
    THORQ_CRYPTO_VERIFY,
    THORQ_CRYPTO_OK,
};

/**
 * @brief thorq_payload_crypto_pack
 * @param payload
 * @param state
 */
inline void thorq_payload_crypto_pack(std::vector<std::uint8_t>& payload, thorq_crypto_cmd_t state)
{
    payload.resize(2);
    payload[0] = THORQ_PAYLOAD_ID_CRYPTO;
    payload[1] = static_cast<std::uint8_t>(state);
}

/**
 * @brief thorq_payload_crypto_pack
 * @param payload
 * @param state
 * @param data
 * @param size
 */
inline void thorq_payload_crypto_pack(std::vector<std::uint8_t>& payload, thorq_crypto_cmd_t state, const quint8* data, std::size_t size)
{
    payload.resize(2 + size);
    payload[0] = THORQ_PAYLOAD_ID_CRYPTO;
    payload[1] = static_cast<std::uint8_t>(state);

    memcpy(payload.data() + 2, data, size);
}

/**
 * @brief thorq_payload_crypto_pack
 * @param payload
 * @param state
 * @param data
 */
inline void thorq_payload_crypto_pack(std::vector<std::uint8_t>& payload, thorq_crypto_cmd_t state, const std::vector<std::uint8_t>& data)
{
    payload.resize(2 + data.size());
    payload[0] = THORQ_PAYLOAD_ID_CRYPTO;
    payload[1] = static_cast<std::uint8_t>(state);

    memcpy(payload.data() + 2, data.data(), data.size());
}

/**
 * @brief thorq_payload_crypto_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_crypto_is_valid(const std::vector<std::uint8_t>& payload)
{
    return payload.size() >= 2 && payload[0] == THORQ_PAYLOAD_ID_CRYPTO;
}

/**
 * @brief thorq_payload_crypto_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_crypto_get_cmd(const std::vector<std::uint8_t>& payload, thorq_crypto_cmd_t& cmd)
{
    cmd = static_cast<thorq_crypto_cmd_t>(payload[1]);
}

/**
 * @brief thorq_payload_crypto_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_crypto_get_data(const std::vector<std::uint8_t>& payload, std::vector<std::uint8_t>& data)
{
    data.resize(payload.size() - 2);

    memcpy(data.data(), payload.data() + 2, payload.size() - 2);
}

#endif // THORQ_PAYLOAD_CRYPTO_H
