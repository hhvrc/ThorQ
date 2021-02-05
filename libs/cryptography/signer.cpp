#include "signer.h"

#include "utils.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

constexpr std::size_t MinFileSize = ThorQ::Crypto::Signer::PublicKeyLen + ThorQ::Crypto::Signer::SignatureLen;
constexpr std::size_t MaxFileSize = MinFileSize + ThorQ::Crypto::Signer::SignatureLen;

const std::array<std::uint8_t, ThorQ::Crypto::Signer::PublicKeyLen> ThorQ::Crypto::Signer::RootSigner()
{
    // This is the root public key to verify key changes commited by the server
    // >>>>>>>>>>> DO NOT REMOVE OR MODIFY <<<<<<<<<<<
    return std::array<std::uint8_t, ThorQ::Crypto::Signer::PublicKeyLen> {
        0xF0, 0x8D, 0xAD, 0x40, 0xA0, 0xAD, 0x7A, 0xAF,
        0x26, 0xF1, 0x38, 0xCB, 0x16, 0x13, 0x4F, 0x22,
        0xF1, 0x05, 0xD2, 0x3D, 0xD4, 0x64, 0xEC, 0x6C,
        0xF2, 0x33, 0xDA, 0x3B, 0x86, 0x83, 0x07, 0x90
    };
}

ThorQ::Crypto::Signer::Signer()
    : m_pk{0}
    , m_sk{0}
    , m_state(State::Uninitialized)
{
    if (sodium_init() < 0) throw "Failed to initialize libsodium";
}

ThorQ::Crypto::Signer::~Signer()
{
    // Zero out memory to not leave a footprint in RAM
    clear();
}

bool ThorQ::Crypto::Signer::trySaveToFile(const char* path, bool onlyPublicKey) const
{
    // Buffer to write to
    std::vector<std::uint8_t> data;
    data.reserve(MaxFileSize);

    // Insert public key
    data.insert(data.begin(), m_pk.begin(), m_pk.end());

    // Insert secret key
    if (!onlyPublicKey && m_state == State::BothKeys) {
        data.insert(data.end(), m_sk.begin(), m_sk.end());
    }

    std::size_t keysSize = data.size();
    data.resize(keysSize + ThorQ::Crypto::Signer::SignatureLen);

    std::uint8_t* keysPtr = data.data();
    std::uint8_t* signaturePtr = keysPtr + keysSize;

    // Create signature of the keypair thats being stored
    if (crypto_sign_detached(signaturePtr, nullptr, keysPtr, keysSize, m_sk.data()) != 0) {
        return false;
    }

    // Write to file
    return tryWriteAll(path, data);
}

bool ThorQ::Crypto::Signer::tryLoadFromFile(const char* path)
{
    // Buffer to write to
    std::vector<std::uint8_t> data;
    data.reserve(MaxFileSize);

    // Read file
    if (!tryReadAll(path, data, MaxFileSize)) {
        return false;
    }

    // Check if the file size is valid (one of the two)
    if (data.size() != MinFileSize && data.size() != MaxFileSize) {
        return false;
    }

    std::size_t keysSize = data.size() - Signer::SignatureLen;
    std::uint8_t* keysPtr = data.data();
    std::uint8_t* signaturePtr = data.data() + keysSize;

    // Check that the file signature is correct (signature, data, datasize, publicKey)
    if (crypto_sign_verify_detached(signaturePtr, keysPtr, keysSize, keysPtr) != 0) {
        return false;
    }

    // Write the data to this
    memcpy(m_pk.data(), data.data(), Signer::PublicKeyLen);

    if (keysSize == Signer::PublicKeyLen) {
        m_state = State::OnlyPublicKey;
    }
    else {
        m_state = State::BothKeys;
        memcpy(m_sk.data(), data.data() + Signer::PublicKeyLen, Signer::SecretKeyLen);
    }

    return true;
}

void ThorQ::Crypto::Signer::clear()
{
    memset(m_pk.data(), 0, ThorQ::Crypto::Signer::PublicKeyLen);
    memset(m_sk.data(), 0, ThorQ::Crypto::Signer::SecretKeyLen);
    m_state = State::Uninitialized;
}

bool ThorQ::Crypto::Signer::generateKeyPair()
{
    if (crypto_sign_keypair(m_pk.data(), m_sk.data()) != 0) {
        return false;
    }
    m_state = State::BothKeys;
    return true;
}

bool ThorQ::Crypto::Signer::setPublicKey(const std::uint8_t* publicKey, std::size_t keySize)
{
    if (keySize != Signer::PublicKeyLen) {
        return false;
    }

    clear();
    memcpy(m_pk.data(), publicKey, keySize);
    m_state = State::OnlyPublicKey;
    return true;
}

bool ThorQ::Crypto::Signer::sign(const std::uint8_t* data, std::size_t dataSize, std::uint8_t* signature, std::size_t signatureSize) const
{
    if (signatureSize != Signer::SignatureLen) {
        return false;
    }

    return crypto_sign_detached(signature, nullptr, data, dataSize, m_sk.data()) == 0;
}

bool ThorQ::Crypto::Signer::verify(const std::uint8_t* data, std::size_t dataSize, const std::uint8_t* signature, std::size_t signatureSize) const
{
    if (signatureSize != Signer::SignatureLen) {
        return false;
    }

    return crypto_sign_verify_detached(signature, data, dataSize, m_pk.data()) == 0;
}
