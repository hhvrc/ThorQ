#include "thorq_message.h"

#include <cstdlib>
#include <cstring>

#include "enums.h"
#include "crypto.h"
#include "constants.h"

// htonl/ntohl
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
#include <WinSock2.h>
#elif __linux__
#include <arpa/inet.h>
#endif

#include "enet/include/enet.h"

struct Content
{
    std::uint8_t  flags;
    std::uint64_t counter;
    std::uint8_t  data[1024 * 64];
    std::uint8_t  hmac[32];
    std::uint8_t  iv[12];
};

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

bool ThorQ::packetEncode(ENetPacket* packet, const std::vector<std::uint8_t>& payload)
{
    if (payload.size() > THORQ_PAYLOAD_LEN_MAX || payload.size() < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    // Set header
    packet->data[0] = 0;

    // Copy data
    memcpy(packet->data + 1, payload.data(), payload.size());

    // Set size
    packet->dataLength = payload.size();

    return true;
}

bool ThorQ::packetEncode(ENetPacket* packet, const std::vector<std::uint8_t>& payload, ThorQ::Crypto* crypto)
{
    if (payload.size() > THORQ_PAYLOAD_LEN_MAX || payload.size() < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    // Set header
    packet->data[0] = PREENCRYPTION_FLAG_ENCRYPTED;

    // Encrpyt the data, this will copy it and the generated IV into messageOut
    if (crypto->encrypt(packet->data + 1, payload.data(), payload.size(), packet->data + 1 + payload.size()))
    {
        return packet;
    }

    return true;
}

bool ThorQ::packetDecode(const ENetPacket* packet, std::vector<std::uint8_t>& payload, ThorQ::Crypto *crypto)
{
    if (packet->dataLength > THORQ_PAYLOAD_LEN_MAX || packet->dataLength < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    payload.resize(packet->dataLength);

    if ((packet->data[0] & PREENCRYPTION_FLAG_ENCRYPTED) != 0)
    {
        if (!crypto->decrypt(payload.data(), packet->data + 1, THORQ_PAYLOAD_LEN_MAX, packet->data + 1 + THORQ_PAYLOAD_LEN_MAX))
        {
            return false;
        }
    }
    else
    {
        memcpy(payload.data(), packet->data + 1, packet->dataLength);
    }

    return true;
}
