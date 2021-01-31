#include "passwordhash.h"

#include <cassert>
#include <array>
#include <cstdint>

inline bool HashPassword(const char* password, std::size_t passwordLen, std::uint8_t* seed, const std::uint8_t* salt) {
    return crypto_pwhash(seed, crypto_box_SEEDBYTES, password, passwordLen, salt, crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE, crypto_pwhash_ALG_DEFAULT) == 0;
}

bool ThorQ::Crypto::PasswordHash::Generate(const char* password, std::size_t passwordLen, std::span<std::uint8_t, PasswordHash::HashLength> hash)
{
    std::uint8_t* hashSalt = hash.data();
    std::uint8_t* hashSeed = hashSalt + crypto_pwhash_SALTBYTES;

    randombytes_buf(hashSalt, crypto_pwhash_SALTBYTES);

    return HashPassword(password, passwordLen, hashSeed, hashSalt);
}

bool ThorQ::Crypto::PasswordHash::Verify(const char* password, std::size_t passwordLen, const std::span<const std::uint8_t, PasswordHash::HashLength> hash)
{
    const std::uint8_t* hashSalt = hash.data();
    const std::uint8_t* hashSeed = hashSalt + crypto_pwhash_SALTBYTES;

    std::array<std::uint8_t, crypto_box_SEEDBYTES> seed;

    return HashPassword(password, passwordLen, seed.data(), hashSalt) &&
           memcmp(seed.data(), hashSeed, crypto_box_SEEDBYTES) == 0;
}
