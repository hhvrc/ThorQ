#include "encoding.h"

#include "hashing.h"
#include "constants.h"
#include "enums.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>

// htonl/ntohl
#if defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
#include <WinSock2.h>
#elif __linux__
#include <arpa/inet.h>
#endif

/// Flags to describe the state of a message
enum PREENCRYPTION_FLAG : std::uint16_t
{
    NONE        = 0,
    ENCRYPTED   = 1 << 0, ///< The following data is encrypted, it needs to get decrypted to make sense
    RESERVED_1  = 1 << 1,
    RESERVED_2  = 1 << 2,
    RESERVED_3  = 1 << 3,
    RESERVED_4  = 1 << 4,
    RESERVED_5  = 1 << 5,
    RESERVED_6  = 1 << 6,
    RESERVED_7  = 1 << 7,
    RESERVED_8  = 1 << 8,
    RESERVED_9  = 1 << 9,
    RESERVED_10 = 1 << 10,
    RESERVED_11 = 1 << 11,
    RESERVED_12 = 1 << 11,
    RESERVED_13 = 1 << 12,
    RESERVED_14 = 1 << 13,
    RESERVED_15 = 1 << 14,
    RESERVED_16 = 1 << 15
};

bool isCorrupted(const ThorQ::Encoding::MessageHeader* header, const std::uint8_t* data, std::uint32_t size) {
    return ntohl(header->checkSum) != ThorQ::Hashing::Crc32(data, size);
}
bool isEncrypted(const ThorQ::Encoding::MessageHeader* header) {
    return (ntohs(header->flags) & PREENCRYPTION_FLAG::ENCRYPTED) != 0;
}
constexpr std::uint32_t PacketOverhead(bool encrypted) {
    return sizeof(ThorQ::Encoding::MessageHeader) + (encrypted ? ThorQ::Crypto::Encryption::DataOverhead : 0);
}
bool ThorQ::Encoding::isHeaderValid(const ThorQ::Encoding::MessageHeader* header) {
    std::uint32_t bodySize = ntohl(header->bodySize);

    return bodySize >= ThorQ::Encoding::MinimumMessageSize - ThorQ::Encoding::HeaderSize &&
           bodySize <= ThorQ::Encoding::MaximumMessageSize - ThorQ::Encoding::HeaderSize;
}
bool ThorQ::Encoding::isMessageValid(const std::uint8_t* data, std::uint32_t size) {
    if (size <= ThorQ::Encoding::MinimumMessageSize || size >= ThorQ::Encoding::MaximumMessageSize) {
        return false;
    }

    std::uint32_t bodySize = size - ThorQ::Encoding::HeaderSize;
    const std::uint8_t* bodyPtr = data + ThorQ::Encoding::HeaderSize;
    const MessageHeader* headerPtr = reinterpret_cast<const MessageHeader*>(data);

    if (isCorrupted(headerPtr, bodyPtr, bodySize)) {
        return false;
    }

    std::uint32_t payloadSize = size - PacketOverhead(isEncrypted(headerPtr));

    return payloadSize >= THORQ_PAYLOAD_LEN_MIN && payloadSize <= THORQ_PAYLOAD_LEN_MAX;
}
std::uint32_t ThorQ::Encoding::calculateMessageSize(std::uint32_t dataSize, bool encrypt) {
    return dataSize + PacketOverhead(encrypt);
}
std::uint32_t ThorQ::Encoding::calculateDataSize(const std::uint8_t* data, std::uint32_t size) {
    if (size < sizeof(MessageHeader)) {
        return 0;
    }

    return size - PacketOverhead(isEncrypted(reinterpret_cast<const MessageHeader*>(data)));
}

bool ThorQ::Encoding::messageEncode(const std::uint8_t* dataIn, std::uint32_t sizeIn, std::uint8_t* dataOut, std::uint32_t sizeOut) {
    std::size_t requiredSize = sizeIn + PacketOverhead(false);

    // Check output bounds
    if (sizeOut != requiredSize) {
        return false;
    }

    // Positions
    std::uint8_t* headerPtr  = dataOut;
    std::uint32_t headerSize = sizeof(MessageHeader);
    std::uint8_t* bodyPtr    = headerPtr + headerSize;
    std::uint32_t bodySize   = requiredSize - headerSize;

    // Copy data
    memcpy(bodyPtr, dataIn, bodySize);

    // Get header
    MessageHeader& header = *reinterpret_cast<MessageHeader*>(dataOut);

    // Set header properties
    header.bodySize = htonl(bodySize);
    header.checkSum = htonl(ThorQ::Hashing::Crc32(bodyPtr, bodySize));
    header.flags    = htons(PREENCRYPTION_FLAG::NONE);

    return true;
}

bool ThorQ::Encoding::messageEncode(const std::uint8_t* dataIn, std::uint32_t sizeIn, std::uint8_t* dataOut, std::uint32_t sizeOut, const ThorQ::Crypto::Encryption& encrypter) {
    std::size_t requiredSize = sizeIn + PacketOverhead(true);

    // Check output bounds
    if (sizeOut != requiredSize) {
        return false;
    }

    // Body bounds
    std::uint8_t* bodyPtr  = dataOut + sizeof(MessageHeader);
    std::uint32_t bodySize = sizeOut - sizeof(MessageHeader);

    // Encrypt data
    if (!encrypter.encrypt(dataIn, sizeIn, bodyPtr, bodySize)) {
        return false;
    }

    // Get header
    MessageHeader& header = *reinterpret_cast<MessageHeader*>(dataOut);

    // Set header properties
    header.bodySize = htonl(bodySize);
    header.checkSum = htonl(ThorQ::Hashing::Crc32(bodyPtr, bodySize));
    header.flags    = htons(PREENCRYPTION_FLAG::ENCRYPTED);

    return true;
}

bool ThorQ::Encoding::messageDecode(const std::uint8_t* dataIn, std::uint32_t sizeIn, std::uint8_t* dataOut, std::uint32_t sizeOut, const ThorQ::Crypto::Encryption& encrypter) {
    // Check header bounds
    if (sizeIn <= sizeof(MessageHeader)) {
        return false;
    }

    // Get header
    const MessageHeader* header = reinterpret_cast<const MessageHeader*>(dataIn);

    // Check output bounds
    if (sizeOut != sizeIn - PacketOverhead(isEncrypted(header))) {
        return false;
    }

    // Body bounds
    const std::uint8_t* payloadPtr  = dataIn + sizeof(MessageHeader);
    const std::uint32_t payloadSize = sizeIn - sizeof(MessageHeader);

    if (!isEncrypted(header)) {
        memcpy(dataOut, payloadPtr, payloadSize);
        return true;
    }

    // Decrypt data
    return encrypter.decrypt(payloadPtr, payloadSize, dataOut, sizeOut);
}

bool ThorQ::Encoding::isMessageEncrypted(const uint8_t *data, uint32_t size)
{
    if (size <= sizeof(MessageHeader)) {
        return false;
    }
    return isEncrypted(reinterpret_cast<const MessageHeader*>(data));
}
