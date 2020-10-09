#include "thorq_message.h"

#include <cstdlib>
#include <cstring>

#include "enums.h"
#include "crypto.h"
#include "constants.h"
#include "thorq_payload.h"

// htonl/ntohl
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
#include <WinSock2.h>
#elif __linux__
#include <arpa/inet.h>
#endif

#include "enet/include/enet.h"

/// Flags to describe the state of a message
enum PREENCRYPTION_FLAG : std::uint8_t
{
    PREENCRYPTION_FLAG_ENCRYPTED  = 1 << 0, ///< The following data is encrypted, it needs to get decrypted to make sense
    PREENCRYPTION_FLAG_RESERVED_2 = 1 << 1,
    PREENCRYPTION_FLAG_RESERVED_3 = 1 << 2,
    PREENCRYPTION_FLAG_RESERVED_4 = 1 << 3,
    PREENCRYPTION_FLAG_RESERVED_5 = 1 << 4,
    PREENCRYPTION_FLAG_RESERVED_6 = 1 << 5,
    PREENCRYPTION_FLAG_RESERVED_7 = 1 << 6,
    PREENCRYPTION_FLAG_RESERVED_8 = 1 << 7,
};

void DestroyENetPacket(void* ptr)
{
    ENetPacket* packet = reinterpret_cast<ENetPacket*>(ptr);

    // Free allocated data
    delete[] packet->data;

    // Free struct
    delete packet;
}
ENetPacket* CreateENetPacket(std::size_t size, std::uint8_t flags)
{
    ENetPacket* packet = new ENetPacket();

    packet->data = new enet_uint8[size];
    packet->dataLength = size;

    packet->flags = flags | ENET_PACKET_FLAG_NO_ALLOCATE;

    packet->freeCallback = &DestroyENetPacket;
    packet->referenceCount = 0;

    return packet;
}

ENetPacket* ThorQ::packetEncode(const ThorQ::Payload& payload, std::uint8_t flags, ThorQ::Crypto* crypto)
{
    if (payload.dataSize() <= THORQ_PAYLOAD_CAP)
    {
        ENetPacket* packet = CreateENetPacket(payload.dataSize(), flags & (ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_UNSEQUENCED));

        // Set data
        if (crypto != nullptr)
        {
            // Set header
            packet->data[0] = PREENCRYPTION_FLAG_ENCRYPTED;

            // Encrpyt the data, this will copy it and the generated IV into messageOut
            if (crypto->encrypt(packet->data + 1, (const std::uint8_t*)&payload, sizeof(THORQ_PAYLOAD), &packet->data[1 + sizeof(THORQ_PAYLOAD)]))
            {
                return packet;
            }
        }
        else
        {
            // Set header
            packet->data[0] = 0;

            memcpy(packet->data + 1, &payload, sizeof(THORQ_PAYLOAD));

            return packet;
        }

        DestroyENetPacket(packet);
    }

    return nullptr;
}

ThorQ::Payload ThorQ::packetDecode(const ENetPacket* packet, ThorQ::Crypto *crypto)
{
    ThorQ::Payload payload;

    if ((packet->data[0] & PREENCRYPTION_FLAG_ENCRYPTED) != 0)
    {
        if (!crypto->decrypt((std::uint8_t*)&payload, packet->data + 1, sizeof(ThorQ::THORQ_PAYLOAD), packet->data + 1 + sizeof(ThorQ::THORQ_PAYLOAD)))
        {
            return {};
        }
    }
    else
    {
        memcpy(&payload, packet->data + 1, sizeof(THORQ_PAYLOAD));
    }

    if (payload.payloadId() > THORQ_PAYLOAD_ID__MAX || payload.dataSize() > THORQ_PAYLOAD_CAP)
    {
        return {};
    }

    return payload;
}
