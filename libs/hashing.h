#ifndef HASHING_H
#define HASHING_H

#include <span>
#include <cstdint>

namespace ThorQ {
namespace Hashing {
std::uint32_t Crc32(const std::span<const std::uint8_t> data);
}
}

#endif // HASHING_H
