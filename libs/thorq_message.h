/// @file thorq_message.h
///
///

#ifndef THORQ_MESSAGE_H
#define THORQ_MESSAGE_H

#include <cstdint>
#include <vector>

#include <flatbuffers/flatbuffers.h>

#include "typedefs_global.h"

namespace ThorQ {
bool packetEncode(ENetPacket* pakcet, const flatbuffers::DetachedBuffer& payload);
bool packetEncode(ENetPacket* pakcet, const flatbuffers::DetachedBuffer& payload, ThorQ::Crypto* crypto);
bool packetDecode(const ENetPacket* packet, std::vector<std::uint8_t>&   payload, ThorQ::Crypto* crypto);
}

#endif // THORQ_MESSAGE_H
