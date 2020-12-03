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

constexpr bool PacketIsEncrypted(const ENetPacket* const packet)
{
    return (packet->data[0] & (std::uint8_t)PREENCRYPTION_FLAG::ENCRYPTED) != 0;
}
constexpr std::size_t PacketOverhead(bool encrypted)
{
    return 1 + (encrypted * (ThorQ::Crypto::MacLen + ThorQ::Crypto::NonceLen));
}

constexpr std::size_t PacketEncMacOffset = 1;
constexpr std::size_t PacketEncNonceOffset = PacketEncMacOffset + ThorQ::Crypto::MacLen;
constexpr std::size_t PacketEncPayloadOffset = PacketEncNonceOffset + ThorQ::Crypto::NonceLen;
constexpr std::size_t PacketRawPayloadOffset = 1;

bool ThorQ::packetIsValidSize(const ENetPacket * const packet)
{
    std::size_t overhead = PacketOverhead(PacketIsEncrypted(packet));

    return packet->dataLength >= THORQ_PAYLOAD_LEN_MIN + overhead &&
           packet->dataLength <= THORQ_PAYLOAD_LEN_MAX + overhead;
}

std::size_t ThorQ::calculateDataSize(const ENetPacket* const packet)
{
    return packet->dataLength - PacketOverhead(PacketIsEncrypted(packet));
}

std::size_t ThorQ::calculatePacketSize(std::size_t dataSize, bool encrypt)
{
    return dataSize + PacketOverhead(encrypt);
}

bool ThorQ::packetEncode(ENetPacket* packet, const std::span<std::uint8_t> data)
{
    std::size_t sizeNeeded = ThorQ::calculatePacketSize(data.size(), false);

    if (packet->dataLength != sizeNeeded ||
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

    // Copy in data
    std::copy(data.begin(), data.end(), packetPayload.begin());

    return true;
}

bool ThorQ::packetEncode(ENetPacket* packet, const std::span<std::uint8_t> data, std::shared_ptr<ThorQ::Crypto> crypto)
{
    std::size_t sizeNeeded = ThorQ::calculatePacketSize(data.size(), true);

    if (packet->dataLength != sizeNeeded ||
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
    if (!crypto->encrypt(packetPayload, data, packetMAC, packetNonce))
    {
        return false;
    }

    return true;
}

bool ThorQ::packetDecode(const ENetPacket* packet, std::span<std::uint8_t> data, std::shared_ptr<ThorQ::Crypto> crypto)
{
    std::size_t sizeNeeded = ThorQ::calculateDataSize(packet);

    if (data.size() != sizeNeeded ||
        data.size() > THORQ_PAYLOAD_LEN_MAX ||
        data.size() < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    // Get data sections
    std::span<std::uint8_t> packetData(packet->data, packet->dataLength);

    if (PacketIsEncrypted(packet))
    {
        // Get data sections
        std::span<std::uint8_t> packetMAC     = packetData.subspan(PacketEncMacOffset,     Crypto::MacLen);
        std::span<std::uint8_t> packetNonce   = packetData.subspan(PacketEncNonceOffset,   Crypto::NonceLen);
        std::span<std::uint8_t> packetPayload = packetData.subspan(PacketEncPayloadOffset, data.size());

        // Decrypt data
        if (!crypto->decrypt(data, packetPayload, packetMAC, packetNonce))
        {
            printf("Decrypt failed\n");
            return false;
        }
    }
    else
    {
        // Get data sections
        std::span<std::uint8_t> packetPayload = packetData.subspan(PacketRawPayloadOffset, data.size());

        // Copy out data
        std::copy(packetPayload.begin(), packetPayload.end(), data.begin());
    }

    return true;
}
