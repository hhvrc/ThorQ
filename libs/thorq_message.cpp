#include "thorq_message.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <execution>

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

/// Flags to describe the state of a message
enum class PREENCRYPTION_FLAG : std::uint8_t
{
    NONE       = 0,
    ENCRYPTED  = 1 << 0, ///< The following data is encrypted, it needs to get decrypted to make sense
    RESERVED_2 = 1 << 1,
    RESERVED_3 = 1 << 2,
    RESERVED_4 = 1 << 3,
    RESERVED_5 = 1 << 4,
    RESERVED_6 = 1 << 5,
    RESERVED_7 = 1 << 6,
    RESERVED_8 = 1 << 7,
};

constexpr std::size_t PacketEncMacOffset = 1;
constexpr std::size_t PacketEncNonceOffset = PacketEncMacOffset + ThorQ::Crypto::MacLen;
constexpr std::size_t PacketEncPayloadOffset = PacketEncNonceOffset + ThorQ::Crypto::NonceLen;
constexpr std::size_t PacketRawPayloadOffset = 1;

constexpr std::size_t ThorQ::calculateDataSize(const ENetPacket* const packet)
{
    std::size_t dataSize = packet->dataLength;

    dataSize -= 1; // PreEncryption flag

    if ((packet->data[0] & (std::uint8_t)PREENCRYPTION_FLAG::ENCRYPTED) != 0)
    {
        dataSize -= ThorQ::Crypto::MacLen + ThorQ::Crypto::NonceLen;
    }

    return dataSize;
}

constexpr std::size_t ThorQ::calculatePacketSize(std::size_t dataSize, bool encrypt)
{
    dataSize += 1; // PreEncryption flag

    if (encrypt)
    {
        dataSize += ThorQ::Crypto::MacLen + ThorQ::Crypto::NonceLen;
    }

    return dataSize;
}

bool ThorQ::packetEncode(ENetPacket* packet, const std::span<std::uint8_t> data)
{
    if (packet->dataLength != calculatePacketSize(data.size(), false) ||
        data.size() > THORQ_PAYLOAD_LEN_MAX ||
        data.size() < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    // Get data sections
    std::span<std::uint8_t> packetData(packet->data, packet->dataLength);
    std::span<std::uint8_t> packetPayload = packetData.subspan(PacketRawPayloadOffset, data.size());

    // Set header
    packetData[0] = (std::uint8_t)PREENCRYPTION_FLAG::NONE;

    // Copy data
    std::copy(packetPayload.begin(), packetPayload.end(), data.begin());

    return true;
}

bool ThorQ::packetEncode(ENetPacket* packet, const std::span<std::uint8_t> data, std::shared_ptr<ThorQ::Crypto> crypto)
{
    if (packet->dataLength != calculatePacketSize(data.size(), true) ||
        data.size() > THORQ_PAYLOAD_LEN_MAX ||
        data.size() < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    // Get data sections
    std::span<std::uint8_t> packetData(packet->data, packet->dataLength);
    std::span<std::uint8_t> packetMAC     = packetData.subspan(PacketEncMacOffset,     Crypto::MacLen);
    std::span<std::uint8_t> packetNonce   = packetData.subspan(PacketEncNonceOffset,   Crypto::NonceLen);
    std::span<std::uint8_t> packetPayload = packetData.subspan(PacketEncPayloadOffset, data.size());

    // Set header
    packetData[0] = (std::uint8_t)PREENCRYPTION_FLAG::ENCRYPTED;

    // Encrpyt the data, this will copy it and the generated IV into messageOut
    if (crypto->encrypt(packetPayload, data, packetMAC, packetNonce))
    {
        return packet;
    }

    return true;
}

bool ThorQ::packetDecode(const ENetPacket* packet, std::vector<std::uint8_t>& data, std::shared_ptr<ThorQ::Crypto> crypto)
{
    std::size_t sizeNeeded = ThorQ::calculateDataSize(packet);

    if (data.size() < sizeNeeded ||
        packet->dataLength > THORQ_PAYLOAD_LEN_MAX ||
        packet->dataLength < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    data.resize(sizeNeeded);

    // Get data sections
    std::span<std::uint8_t> packetData(packet->data, packet->dataLength);

    if ((packet->data[0] & (std::uint8_t)PREENCRYPTION_FLAG::ENCRYPTED) != 0)
    {
        // Get data sections
        std::span<std::uint8_t> packetMAC     = packetData.subspan(PacketEncMacOffset,     Crypto::MacLen);
        std::span<std::uint8_t> packetNonce   = packetData.subspan(PacketEncNonceOffset,   Crypto::NonceLen);
        std::span<std::uint8_t> packetPayload = packetData.subspan(PacketEncPayloadOffset, data.size());

        if (!crypto->decrypt(data, packetPayload, packetMAC, packetNonce))
        {
            return false;
        }
    }
    else
    {
        // Get data sections
        std::span<std::uint8_t> packetPayload = packetData.subspan(PacketRawPayloadOffset, data.size());

        std::copy(data.begin(), data.end(), packetPayload.begin());
    }

    return true;
}
