#ifndef THORQ_PAYLOAD_CRYPTO_H
#define THORQ_PAYLOAD_CRYPTO_H

#include "thorq_payload.h"

typedef enum {
    THORQ_CRYPTO_REQUEST,
    THORQ_CRYPTO_ESTABLISH,
    THORQ_CRYPTO_VERIFY,
    THORQ_CRYPTO_OK,
} thorq_crypto_cmd_t;

inline void thorq_payload_crypto_pack(thorq_payload_t& payload, const thorq_crypto_cmd_t& state, const std::uint8_t* data, std::size_t size)
{
	payload.id = THORQ_PAYLOAD_ID_CRYPTO;
	payload.data.resize(1 + size);
	payload.data[0] = state;
	memcpy(&payload.data[1], &data[0], size);
}
inline void thorq_payload_crypto_pack(thorq_payload_t& payload, const thorq_crypto_cmd_t& state, const std::vector<std::uint8_t>& data = std::vector<std::uint8_t>())
{
	payload.id = THORQ_PAYLOAD_ID_CRYPTO;
	payload.data.resize(1 + data.size());
	payload.data[0] = state;
	memcpy(&payload.data[1], &data[0], data.size());
}

inline bool thorq_payload_crypto_is_valid(const thorq_payload_t& payload)
{
	return payload.id == THORQ_PAYLOAD_ID_CRYPTO;
}

inline thorq_crypto_cmd_t thorq_payload_crypto_get_cmd(const thorq_payload_t& payload)
{
	return (thorq_crypto_cmd_t)payload.data[0];
}

inline std::vector<std::uint8_t> thorq_payload_crypto_get_data(const thorq_payload_t& payload)
{
	return std::vector<std::uint8_t>(payload.data.begin() + 1, payload.data.end());
}

#endif // THORQ_PAYLOAD_CRYPTO_H
