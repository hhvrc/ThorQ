#include "encoding.h"

#include <cryptography/encryption.h>
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

constexpr bool IsDataEncrypted(const std::span<const std::uint8_t> packet)
{
    return (packet.data()[0] & (std::uint8_t)PREENCRYPTION_FLAG::ENCRYPTED) != 0;
}
constexpr std::size_t PacketOverhead(bool encrypted)
{
    return 1 + (encrypted * (ThorQ::Crypto::Encryption::Encryption::MacLen + ThorQ::Crypto::Encryption::Encryption::NonceLen));
}

bool ThorQ::Encoding::validateEncodedData(const std::span<const std::uint8_t> data)
{
    std::size_t overhead = PacketOverhead(IsDataEncrypted(data));

    return data.size() >= THORQ_PAYLOAD_LEN_MIN + overhead &&
           data.size() <= THORQ_PAYLOAD_LEN_MAX + overhead;
}

std::size_t ThorQ::Encoding::calculateEncodedSize(std::size_t size, bool encrypt)
{
    return size + PacketOverhead(encrypt);
}

std::size_t ThorQ::Encoding::calculateDecodedSize(const std::span<const std::uint8_t> data)
{
    return data.size() - PacketOverhead(IsDataEncrypted(data));
}

bool ThorQ::Encoding::dataEncode(const std::span<const std::uint8_t> in, std::span<std::uint8_t> out)
{
    std::size_t sizeNeeded = calculateEncodedSize(in.size(), false);

    if (out.size() != sizeNeeded ||
        in.size() > THORQ_PAYLOAD_LEN_MAX ||
        in.size() < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    // Set header
    out[0] = (std::uint8_t)PREENCRYPTION_FLAG::NONE;

    // Copy in data
    std::copy(in.begin(), in.end(), out.begin() + 1);

    return true;
}

bool ThorQ::Encoding::dataEncode(const std::span<const std::uint8_t> in, std::span<std::uint8_t> out, const ThorQ::Crypto::Encryption &crypto)
{
    std::size_t sizeNeeded = calculateEncodedSize(in.size(), true);

    if (out.size() != sizeNeeded ||
        in.size() > THORQ_PAYLOAD_LEN_MAX ||
        in.size() < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    auto it = out.begin();

    // Set header
    *it++ = (std::uint8_t)PREENCRYPTION_FLAG::ENCRYPTED;

    // Get data sections
    std::span<std::uint8_t> packetPayload(it, in.size());
    // it += in.size();
    std::span<std::uint8_t, ThorQ::Crypto::Encryption::MacLen> packetMAC(packetPayload.end(), ThorQ::Crypto::Encryption::MacLen);
    // it += Crypto::MacLen;
    std::span<std::uint8_t, ThorQ::Crypto::Encryption::NonceLen> packetNonce(packetMAC.end(), ThorQ::Crypto::Encryption::NonceLen);
    // it += Crypto::NonceLen;

    // Encrpyt the data, this will copy it and the generated IV into messageOut
    if (!crypto.encrypt(packetPayload, in, packetMAC, packetNonce))
    {
        return false;
    }

    return true;
}

bool ThorQ::Encoding::dataDecode(const std::span<const std::uint8_t> in, std::span<std::uint8_t> out, const ThorQ::Crypto::Encryption &crypto)
{
    std::size_t sizeNeeded = calculateDecodedSize(in);

    if (sizeNeeded != out.size() ||
        sizeNeeded >  THORQ_PAYLOAD_LEN_MAX ||
        sizeNeeded <  THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }

    auto it = in.begin() + 1;

    if (IsDataEncrypted(in))
    {
        // Get data sections
        const std::span<const std::uint8_t> packetPayload(it, sizeNeeded);
        const std::span<const std::uint8_t, ThorQ::Crypto::Encryption::MacLen> packetMAC(packetPayload.end(), ThorQ::Crypto::Encryption::MacLen);
        const std::span<const std::uint8_t, ThorQ::Crypto::Encryption::NonceLen> packetNonce(packetMAC.end(), ThorQ::Crypto::Encryption::NonceLen);

        // Decrypt data
        if (!crypto.decrypt(out, packetPayload, packetMAC, packetNonce))
        {
            printf("Decrypt failed\n");
            return false;
        }
    }
    else
    {
        // Copy out data
        std::copy(it, in.end(), out.begin());
    }

    return true;
}
