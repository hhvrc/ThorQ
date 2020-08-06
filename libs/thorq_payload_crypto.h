#ifndef THORQ_PAYLOAD_CRYPTO_H
#define THORQ_PAYLOAD_CRYPTO_H

#include "thorq_payload.h"

/** @file thorq_payload_crypto.h
 *
 */

/** @typedef thorq_crypto_cmd_t
 *
 */
typedef enum {
    THORQ_CRYPTO_REQUEST,
    THORQ_CRYPTO_ESTABLISH,
    THORQ_CRYPTO_VERIFY,
    THORQ_CRYPTO_OK,
} thorq_crypto_cmd_t;

/**
 * @brief thorq_payload_crypto_pack
 * @param payload
 * @param state
 */
inline void thorq_payload_crypto_pack(thorq_payload_t& payload, thorq_crypto_cmd_t state)
{
    payload.id = THORQ_PAYLOAD_ID_CRYPTO;
    payload.data.resize(1);
    payload.data[0] = static_cast<std::uint8_t>(state);
}

/**
 * @brief thorq_payload_crypto_pack
 * @param payload
 * @param state
 * @param data
 * @param size
 */
inline void thorq_payload_crypto_pack(thorq_payload_t& payload, thorq_crypto_cmd_t state, const std::uint8_t* data, std::size_t size)
{
	payload.id = THORQ_PAYLOAD_ID_CRYPTO;
	payload.data.resize(1 + size);
    payload.data[0] = static_cast<std::uint8_t>(state);
    if (size != 0)
        memcpy(&payload.data[1], &data[0], size);
}

/**
 * @brief thorq_payload_crypto_pack
 * @param payload
 * @param state
 * @param data
 */
inline void thorq_payload_crypto_pack(thorq_payload_t& payload, thorq_crypto_cmd_t state, const std::vector<std::uint8_t>& data)
{
	payload.id = THORQ_PAYLOAD_ID_CRYPTO;
	payload.data.resize(1 + data.size());
    payload.data[0] = static_cast<std::uint8_t>(state);
    if (data.size() != 0)
        memcpy(&payload.data[1], &data[0], data.size());
}

/**
 * @brief thorq_payload_crypto_is_valid
 * @param payload
 * @return
 */
inline bool thorq_payload_crypto_is_valid(const thorq_payload_t& payload)
{
	return payload.id == THORQ_PAYLOAD_ID_CRYPTO;
}

/**
 * @brief thorq_payload_crypto_get_cmd
 * @param payload
 * @param cmd
 */
inline void thorq_payload_crypto_get_cmd(const thorq_payload_t& payload, thorq_crypto_cmd_t& cmd)
{
    cmd = static_cast<thorq_crypto_cmd_t>(payload.data[0]);
}

/**
 * @brief thorq_payload_crypto_get_data
 * @param payload
 * @param data
 */
inline void thorq_payload_crypto_get_data(const thorq_payload_t& payload, std::vector<std::uint8_t>& data)
{
    data.resize(payload.data.size() - 1);
    if (data.size() != 0)
        memcpy(&data[0], &payload.data[1], payload.data.size() - 1);
}

#endif // THORQ_PAYLOAD_CRYPTO_H
