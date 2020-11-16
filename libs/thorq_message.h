/// @file thorq_message.h
///
///

#ifndef THORQ_MESSAGE_H
#define THORQ_MESSAGE_H

#include <cstdint>
#include <vector>
#include <memory>
#include <span>

#include <flatbuffers/flatbuffers.h>

#include "typedefs_global.h"

namespace ThorQ {
constexpr std::size_t calculateDataSize(const ENetPacket* const pakcet);
constexpr std::size_t calculatePacketSize(std::size_t dataSize, bool encrypt);
bool packetEncode(ENetPacket* pakcet, const std::span<std::uint8_t> data);
bool packetEncode(ENetPacket* pakcet, const std::span<std::uint8_t> data, std::shared_ptr<ThorQ::Crypto> crypto);
bool packetDecode(const ENetPacket* packet, std::vector<std::uint8_t>& data, std::shared_ptr<ThorQ::Crypto> crypto);
}

#endif // THORQ_MESSAGE_H
