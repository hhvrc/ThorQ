#ifndef HASHING_H
#define HASHING_H

#include <span>
#include <cstdint>

namespace ThorQ::Hashing {
std::uint32_t Crc32(const std::uint8_t* data, std::size_t size);
std::uint32_t Crc32c(const std::uint8_t* data, std::size_t size);
}

#endif // HASHING_H
