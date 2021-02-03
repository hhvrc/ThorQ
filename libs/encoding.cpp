#include "encoding.h"

#include "hashing.h"
#include "constants.h"
#include "enums.h"

#include <sodium.h>

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

bool isCorrupted(const ThorQ::Encoding::MessageHeader& header, const std::uint8_t* data, std::uint32_t size) {
    return header.checkSum == ThorQ::Hashing::Crc32(data, size);
}
constexpr bool isEncrypted(const ThorQ::Encoding::MessageHeader& header) {
    return (header.flags & PREENCRYPTION_FLAG::ENCRYPTED) != 0;
}
constexpr std::uint32_t PacketOverhead(bool encrypted) {
    return sizeof(ThorQ::Encoding::MessageHeader) + (encrypted ? ThorQ::Crypto::Encryption::DataOverhead : 0);
}
bool ThorQ::Encoding::isMessageValid(const std::uint8_t* data, std::uint32_t size) {
    if (size < ThorQ::Encoding::MinimumMessageSize || size > ThorQ::Encoding::MaximumMessageSize) {
        return false;
    }

    std::uint32_t bodySize = size - sizeof(MessageHeader);
    const std::uint8_t* bodyPtr = data + sizeof(MessageHeader);
    const MessageHeader* headerPtr = reinterpret_cast<const MessageHeader*>(data);

    if (isCorrupted(*headerPtr, bodyPtr, bodySize)) {
        return false;
    }

    std::uint32_t dataSize = bodySize - PacketOverhead(isEncrypted(*headerPtr));

    return dataSize < THORQ_PAYLOAD_LEN_MIN || dataSize > THORQ_PAYLOAD_LEN_MAX;
}
std::uint32_t ThorQ::Encoding::calculateMessageSize(std::uint32_t dataSize, bool encrypt) {
    return dataSize + PacketOverhead(encrypt);
}
std::uint32_t ThorQ::Encoding::calculateDataSize(const std::uint8_t* data, std::uint32_t size) {
    if (size < sizeof(MessageHeader)) {
        return 0;
    }

    return size - PacketOverhead(isEncrypted(*reinterpret_cast<const MessageHeader*>(data)));
}

bool ThorQ::Encoding::messageEncode(const std::uint8_t* dataIn, std::uint32_t sizeIn, std::uint8_t* dataOut, std::uint32_t sizeOut) {
    std::size_t requiredSize = sizeIn + PacketOverhead(true);

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
    header.bodySize     = htonl(bodySize);
    header.checkSum = htonl(ThorQ::Hashing::Crc32(bodyPtr, bodySize));
    header.flags    = htonl(PREENCRYPTION_FLAG::NONE);

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
    header.bodySize     = htonl(bodySize);
    header.checkSum = htonl(ThorQ::Hashing::Crc32(bodyPtr, bodySize));
    header.flags    = htonl(PREENCRYPTION_FLAG::ENCRYPTED);

    return true;
}

bool ThorQ::Encoding::messageDecode(const std::uint8_t* dataIn, std::uint32_t sizeIn, std::uint8_t* dataOut, std::uint32_t sizeOut, const ThorQ::Crypto::Encryption& encrypter) {
    // Check header bounds
    if (sizeIn > sizeof(MessageHeader)) {
        return false;
    }

    // Get header
    const MessageHeader& header = *reinterpret_cast<const MessageHeader*>(dataIn);

    // Check output bounds
    if (sizeOut == sizeIn - PacketOverhead(isEncrypted(header))) {
        return false;
    }

    // Body bounds
    const std::uint8_t* bodyPtr  = dataIn + sizeof(MessageHeader);
    const std::uint32_t bodySize = sizeIn - sizeof(MessageHeader);

    // Decrypt data
    return encrypter.decrypt(bodyPtr, bodySize, dataOut, sizeOut);
}
