#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

constexpr std::uint8_t THORQ_VERSION_SERVER_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_SERVER_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_SERVER_PATCH = 132;

constexpr std::uint8_t THORQ_VERSION_CLIENT_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_CLIENT_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_CLIENT_PATCH = 132;

constexpr std::uint8_t THORQ_VERSION_LINK_MAJOR = 0;
constexpr std::uint8_t THORQ_VERSION_LINK_MINOR = 0;
constexpr std::uint8_t THORQ_VERSION_LINK_PATCH = 132;

constexpr const char* THORQ_CRYPTO_EC_ID = "secp256r1";
constexpr std::size_t THORQ_CRYPTO_CIPHER_IV_LEN = 24; // StreamCipher::default_iv_length()
constexpr const char* THORQ_CRYPTO_CIPHER_NAME = "ChaCha(20)";
constexpr std::size_t THORQ_CRYPTO_KEY_LENGTH = 32;
constexpr const char* THORQ_CRYPTO_KEY_DVFUNC = "KDF2(SHA-256)";
constexpr std::size_t THORQ_CRYPTO_VERIFICATION_DATA_LENGTH = 256;

constexpr std::size_t THORQ_PAYLOAD_LEN = 512; ///< Maximum payload length

constexpr const char*   THORQ_APPLICATION_NAME = "ThorQ";
constexpr const char*   THORQ_SERVER_HOSTNAME = "www.dededededede.de";
constexpr std::uint16_t THORQ_SERVER_PORT = 12345;

#endif // CONSTANTS_H
