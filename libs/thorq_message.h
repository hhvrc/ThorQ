/// @file thorq_message.h
///
///

#ifndef THORQ_MESSAGE_H
#define THORQ_MESSAGE_H

#include <cstdint>

#include "typedefs_global.h"

namespace ThorQ {
ENetPacket* packetEncode(const ThorQ::Payload& payload, std::uint8_t flags, ThorQ::Crypto* crypto = nullptr);
ThorQ::Payload packetDecode(const ENetPacket* packet, ThorQ::Crypto* crypto);
}

#endif // THORQ_MESSAGE_H
