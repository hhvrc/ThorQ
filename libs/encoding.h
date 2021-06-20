#ifndef ENCODING_H
#define ENCODING_H

#include <cryptography/encryption.h>
#include <constants.h>

#include <span>
#include <vector>
#include <memory>
#include <cstdint>

namespace ThorQ::Encoding {

#pragma pack(push, 1)
struct MessageHeader
{
    std::uint32_t bodySize;
    std::uint32_t checkSum;
    std::uint16_t flags;
};
#pragma pack(pop)

constexpr std::uint32_t HeaderSize = sizeof(MessageHeader);
constexpr std::uint32_t MinimumMessageSize = HeaderSize + THORQ_PAYLOAD_LEN_MIN;
constexpr std::uint32_t TypicalMessageSize = HeaderSize + THORQ_PAYLOAD_LEN_TYP + ThorQ::Crypto::Encryption::DataOverhead;
constexpr std::uint32_t MaximumMessageSize = HeaderSize + THORQ_PAYLOAD_LEN_MAX + ThorQ::Crypto::Encryption::DataOverhead;

bool isHeaderValid(const MessageHeader* header);
bool isMessageValid(const std::uint8_t* data, std::uint32_t size);
bool isMessageEncrypted(const std::uint8_t* data, std::uint32_t size);
std::uint32_t calculateMessageSize(std::uint32_t size, bool encrypt);
std::uint32_t calculateDataSize(const std::uint8_t* data, std::uint32_t size);

bool messageEncode(const std::uint8_t* dataIn, std::uint32_t sizeIn, std::uint8_t* dataOut, std::uint32_t sizeOut);
bool messageEncode(const std::uint8_t* dataIn, std::uint32_t sizeIn, std::uint8_t* dataOut, std::uint32_t sizeOut, const ThorQ::Crypto::Encryption& encrypter);
bool messageDecode(const std::uint8_t* dataIn, std::uint32_t sizeIn, std::uint8_t* dataOut, std::uint32_t sizeOut, const ThorQ::Crypto::Encryption& encrypter);
}

#endif // ENCODING_H
