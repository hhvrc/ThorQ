/// @file thorq_payload_message.h
///
///

#ifndef THORQ_MESSAGE_H
#define THORQ_MESSAGE_H

#include <cstdint>
#include <optional>

#include "enums.h"
#include "constants.h"
#include "typedefs_global.h"

namespace ThorQ {
#pragma pack(push, 1)
typedef struct _THORQ_PAYLOAD
{
    union PayloadData
    {
        std::uint8_t raw[THORQ_PAYLOAD_CAP];
    };

    THORQ_PAYLOAD_ID payloadId() const;
    void setPayloadId(THORQ_PAYLOAD_ID payloadId);

    PayloadData& data();
    const PayloadData& data() const;

    std::uint32_t dataSize() const;
    void setDataSize(std::uint32_t size);
private:
    std::uint32_t m_size = 0;
    std::uint8_t m_id = THORQ_PAYLOAD_ID__INVALID;
    PayloadData m_data{0};

} THORQ_PAYLOAD;
#pragma pack(pop)

ENetPacket* packetEncode(const THORQ_PAYLOAD& payload, std::uint8_t flags, ThorQ::Crypto* crypto = nullptr);
std::optional<THORQ_PAYLOAD> packetDecode(const ENetPacket* packet, ThorQ::Crypto* crypto);
}

#endif // THORQ_MESSAGE_H
