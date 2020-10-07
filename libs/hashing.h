#ifndef HASHING_H
#define HASHING_H

#include <string>
#include <cstddef>
#include <cstdint>

namespace ThorQ {
namespace Hashing {
bool Sha256_Hash(std::uint8_t* data, std::size_t size, std::uint8_t* hashOut);
std::string BCrypt_Hash(const std::string& password);
bool BCrypt_Verify(const std::string& password, const std::string& hash);
}
}

#endif // HASHING_H
