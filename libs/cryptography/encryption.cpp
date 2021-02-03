#include "encryption.h"

#include "utils.h"

#include <algorithm>
#include <cstring>
#include <cstdint>
#include <limits>

constexpr std::size_t MinFileSize = ThorQ::Crypto::Encryption::PublicKeyLen;
constexpr std::size_t MaxFileSize = MinFileSize + ThorQ::Crypto::Encryption::SecretKeyLen;

ThorQ::Crypto::Encryption::Encryption()
    : m_pk{0}
    , m_sk{0}
    , m_fk{0}
{
    if (sodium_init() < 0) throw "Failed to initialize libsodium";
}

ThorQ::Crypto::Encryption::~Encryption()
{
    // Zero out memory to not leave a footprint in RAM
    clear();
}

bool ThorQ::Crypto::Encryption::trySaveToFile(const char *path, bool onlyPublicKey) const
{
    // Buffer to write to
    std::vector<std::uint8_t> data;
    data.reserve(MaxFileSize);

    // Insert public key
    data.insert(data.begin(), m_pk.begin(), m_pk.end());

    // Insert secret key
    if (!onlyPublicKey) {
        data.insert(data.end(), m_sk.begin(), m_sk.end());
    }

    // Write to file
    return tryWriteAll(path, data);
}

bool ThorQ::Crypto::Encryption::tryLoadFromFile(const char *path)
{
    // Buffer to write to
    std::vector<std::uint8_t> data;
    data.reserve(MaxFileSize);

    // Read file
    if (!tryReadAll(path, data, MaxFileSize)) {
        return false;
    }

    // Check if the file size is valid (one of the two)
    if (data.size() == MinFileSize) {
        memcpy(m_pk.data(), data.data(), Encryption::PublicKeyLen);
    }
    else if (data.size() == MaxFileSize) {
        memcpy(m_pk.data(), data.data(), Encryption::PublicKeyLen);
        memcpy(m_sk.data(), data.data() + Encryption::PublicKeyLen, Encryption::SecretKeyLen);
    }
    else {
        return false;
    }

    return true;
}

void ThorQ::Crypto::Encryption::clear()
{
    std::memset(m_pk.data(), 0, m_pk.size());
    std::memset(m_sk.data(), 0, m_sk.size());
    clearForgeinKey();
}

void ThorQ::Crypto::Encryption::clearForgeinKey()
{
    std::memset(m_fk.data(), 0, m_fk.size());
}

bool ThorQ::Crypto::Encryption::generateKeyPair()
{
    clearForgeinKey();

    if (crypto_box_keypair(m_pk.data(), m_sk.data()) != 0) {
        clear();

        return false;
    }

    return true;
}

bool ThorQ::Crypto::Encryption::setForeignKey(const std::uint8_t* publicKey, std::size_t keySize)
{
    if (keySize != Encryption::PublicKeyLen) {
        return false;
    }

    memcpy(m_fk.data(), publicKey, Encryption::PublicKeyLen);

    return true;
}

bool ThorQ::Crypto::Encryption::encrypt(const std::uint8_t* inData, std::size_t inSize, std::uint8_t* outData, std::size_t outSize) const
{
    std::size_t contentSize = inSize;
    std::size_t encryptedSize = outSize;

    if (contentSize + Encryption::DataOverhead != encryptedSize) {
        return false;
    }

    std::uint8_t* macPtr = outData + contentSize;
    std::uint8_t* noncePtr = macPtr + Encryption::MacLen;

    randombytes_buf(noncePtr, Encryption::NonceLen);

    return crypto_box_detached(outData, macPtr, inData, inSize, noncePtr, m_fk.data(), m_sk.data()) == 0;
}

bool ThorQ::Crypto::Encryption::decrypt(const std::uint8_t* inData, std::size_t inSize, std::uint8_t* outData, std::size_t outSize) const
{
    std::size_t contentSize = outSize;
    std::size_t encryptedSize = inSize;

    if (contentSize + Encryption::DataOverhead != encryptedSize) {
        return false;
    }

    const std::uint8_t* macPtr = inData + contentSize;
    const std::uint8_t* noncePtr = macPtr + Encryption::MacLen;

    return crypto_box_open_detached(outData, inData, macPtr, contentSize, noncePtr, m_fk.data(), m_sk.data()) == 0;
}
