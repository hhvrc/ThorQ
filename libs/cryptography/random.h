#ifndef RANDOM_H
#define RANDOM_H

#include <span>
#include <cstdint>

namespace ThorQ {
namespace Crypto {
void RandomizeBytes(std::span<std::uint8_t> bytes);
}
}

#endif // RANDOM_H
