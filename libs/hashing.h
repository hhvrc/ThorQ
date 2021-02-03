#ifndef HASHING_H
#define HASHING_H

#include <span>
#include <cstdint>

namespace ThorQ {
namespace Hashing {
std::uint32_t Crc32(const std::uint8_t* data, std::size_t size);
inline std::uint32_t Crc32(const std::span<const std::uint8_t> data) { return Crc32(data.data(), data.size()); };
}
}

#endif // HASHING_H
