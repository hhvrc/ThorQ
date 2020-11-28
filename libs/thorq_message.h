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
std::size_t calculateDataSize(const ENetPacket* const pakcet);
std::size_t calculatePacketSize(std::size_t dataSize, bool encrypt);
bool packetEncode(ENetPacket* pakcet, const std::span<std::uint8_t> data);
bool packetEncode(ENetPacket* pakcet, const std::span<std::uint8_t> data, std::shared_ptr<ThorQ::Crypto> crypto);
bool packetDecode(const ENetPacket* packet, std::span<std::uint8_t> data, std::shared_ptr<ThorQ::Crypto> crypto);
}

#endif // THORQ_MESSAGE_H
