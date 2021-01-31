#ifndef PASSWORDHASHING_H
#define PASSWORDHASHING_H

#include <sodium.h>

#include <span>
#include <cstdint>

namespace ThorQ {
namespace Crypto {
namespace PasswordHash {
constexpr std::size_t HashLength = crypto_box_SEEDBYTES + crypto_pwhash_SALTBYTES;
bool Generate(const char* password, std::size_t passwordLen, std::span<std::uint8_t, PasswordHash::HashLength> hash);
bool Verify(const char* password, std::size_t passwordLen, const std::span<const std::uint8_t, PasswordHash::HashLength> hash);
}
}
}

#endif // PASSWORDHASHING_H
