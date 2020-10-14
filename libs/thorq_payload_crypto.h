/// @file thorq_payload_crypto.h
///
///

#ifndef THORQ_PAYLOAD_CRYPTO_H
#define THORQ_PAYLOAD_CRYPTO_H

#include <vector>
#include <QByteArray>

#include "enums.h"
#include "crypto.h"
#include "constants.h"
#include "serialization.h"

enum THORQ_PAYLOAD_CRYPTO
{
    THORQ_PAYLOAD_CRYPTO_REQUEST,
    THORQ_PAYLOAD_CRYPTO_ESTABLISH,
    THORQ_PAYLOAD_CRYPTO_VERIFY,
    THORQ_PAYLOAD_CRYPTO_OK,
};

inline bool thorq_payload_crypto_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_CRYPTO) return false;

    if (payload.size() >= 2)
    {
        switch (payload[1]) {
        case THORQ_PAYLOAD_CRYPTO_REQUEST:
        case THORQ_PAYLOAD_CRYPTO_OK:
            return payload.size() == 2;
        case THORQ_PAYLOAD_CRYPTO_ESTABLISH:
            return payload.size() == 2 + CRYPTO_ECDH_PUBLIC_KEY_LEN;
        case THORQ_PAYLOAD_CRYPTO_VERIFY:
            return payload.size() == 2 + THORQ_CRYPTO_VERIFICATION_DATA_LEN;
        default:
            return false;
        }
    }

    return false;
}

inline void thorq_payload_crypto_get_cmd(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_CRYPTO& cmd)
{
    cmd = static_cast<THORQ_PAYLOAD_CRYPTO>(payload[1]);
}

inline void thorq_payload_crypto_request_pack(std::vector<std::uint8_t>& payload)
{
    payload.resize(2);
    payload[0] = THORQ_PAYLOAD_ID_CRYPTO;
    payload[1] = THORQ_PAYLOAD_CRYPTO_REQUEST;
}

inline void thorq_payload_crypto_establish_pack(std::vector<std::uint8_t>& payload, const std::vector<std::uint8_t>& data)
{
    payload.resize(2 + data.size());
    payload[0] = THORQ_PAYLOAD_ID_CRYPTO;
    payload[1] = THORQ_PAYLOAD_CRYPTO_ESTABLISH;

    memcpy(payload.data() + 2, data.data(), data.size());
}
inline void thorq_payload_crypto_establish_unpack(const std::vector<std::uint8_t>& payload, std::vector<std::uint8_t>& data)
{
    data.resize(payload.size() - 2);
    memcpy(data.data(), payload.data() + 2, payload.size() - 2);
}

inline void thorq_payload_crypto_verify_pack(std::vector<std::uint8_t>& payload, const std::vector<std::uint8_t>& data)
{
    payload.resize(2 + data.size());
    payload[0] = THORQ_PAYLOAD_ID_CRYPTO;
    payload[1] = THORQ_PAYLOAD_CRYPTO_VERIFY;

    memcpy(payload.data() + 2, data.data(), data.size());
}
inline void thorq_payload_crypto_verify_unpack(const std::vector<std::uint8_t>& payload, std::vector<std::uint8_t>& data)
{
    data.resize(payload.size() - 2);
    memcpy(data.data(), payload.data() + 2, payload.size() - 2);
}

inline void thorq_payload_crypto_ok_pack(std::vector<std::uint8_t>& payload)
{
    payload.resize(2);
    payload[0] = THORQ_PAYLOAD_ID_CRYPTO;
    payload[1] = THORQ_PAYLOAD_CRYPTO_OK;
}

#endif // THORQ_PAYLOAD_CRYPTO_H
