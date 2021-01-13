/// @file thorq_message.h
///
///

#ifndef THORQ_MESSAGE_H
#define THORQ_MESSAGE_H

#include <span>
#include <vector>
#include <memory>
#include <cstdint>

#include "typedefs_global.h"

namespace ThorQ {
bool packetIsValidSize(const ENetPacket* const packet);
std::size_t calculateDataSize(const ENetPacket* const packet);
std::size_t calculatePacketSize(std::size_t dataSize, bool encrypt);
bool packetEncode(ENetPacket* packet, const std::span<std::uint8_t> data);
bool packetEncode(ENetPacket* packet, const std::span<std::uint8_t> data, const ThorQ::Crypto& crypto);
bool packetDecode(const ENetPacket* packet, std::span<std::uint8_t> data, const ThorQ::Crypto& crypto);
}

#endif // THORQ_MESSAGE_H
