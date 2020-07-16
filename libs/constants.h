#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <cstring>

constexpr std::uint8_t VERSION_MAJOR = 1;
constexpr std::uint8_t VERSION_MINOR = 0;
constexpr std::uint8_t VERSION_PATCH = 0;

constexpr std::size_t MESSAGE_HEADER_SIZE = 8;
constexpr std::size_t MESSAGE_PAYLOAD_MAX = UINT8_MAX;
constexpr std::size_t MESSAGE_DATA_MAX = MESSAGE_HEADER_SIZE + MESSAGE_PAYLOAD_MAX;

#endif // CONSTANTS_H
