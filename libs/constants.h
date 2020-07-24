#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <cstring>

constexpr std::uint8_t THORQ_VERSION_SERVER_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_SERVER_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_SERVER_PATCH = 130;

constexpr std::uint8_t THORQ_VERSION_CLIENT_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_CLIENT_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_CLIENT_PATCH = 130;

constexpr std::uint8_t THORQ_VERSION_LINK_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_LINK_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_LINK_PATCH = 130;

constexpr const char* THORQ_CRYPTO_EC_ID = "secp256r1";
constexpr std::size_t THORQ_CRYPTO_CIPHER_IV_LEN = 24; // StreamCipher::default_iv_length()
constexpr const char* THORQ_CRYPTO_CIPHER_NAME = "ChaCha(20)";
constexpr std::size_t THORQ_CRYPTO_KEY_LENGTH = 32;
constexpr const char* THORQ_CRYPTO_KEY_DVFUNC = "KDF2(SHA-256)";

constexpr std::size_t THORQ_MSG_MAX_PAYLOAD_LEN = 256; ///< Maximum payload length

#endif // CONSTANTS_H
