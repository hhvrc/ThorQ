#include "signing.h"

#include "cryptography/random.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <fstream>

inline bool tryWriteAll(const std::filesystem::path& path, const std::vector<std::uint8_t>& data)
{
    if (std::filesystem::exists(path) && !std::filesystem::remove(path)) {
        return false;
    }

    try {
        std::fstream writer(path, std::ios::out | std::ios::binary);

        if (!writer.is_open()) {
            return false;
        }

        writer.seekg(0, std::ios::beg);

        writer.write((char*)data.data(), data.size());

        writer.close();
    }
    catch (...) {
        return false;
    }

    return true;
}
inline bool tryReadAll(const std::filesystem::path& path, std::vector<std::uint8_t>& data, std::size_t sizeMax = SIZE_MAX)
{
    try {
        std::fstream reader(path, std::ios::in | std::ios::binary);

        if (!reader.is_open()) {
            return false;
        }

        reader.seekg(0, std::ios::end);
        std::size_t size = reader.tellg();
        reader.seekg(0, std::ios::beg);

        if (size > sizeMax) {
            return false;
        }

        data.resize(size);
        reader.read((char*)data.data(), data.size());

        reader.close();
    }
    catch (...) {
        return false;
    }

    return true;
}

constexpr std::size_t MinFileSize = ThorQ::Crypto::Signing::PublicKeyLen + ThorQ::Crypto::Signing::SignatureLen;
constexpr std::size_t MaxFileSize = ThorQ::Crypto::Signing::PublicKeyLen + ThorQ::Crypto::Signing::SecretKeyLen + ThorQ::Crypto::Signing::SignatureLen;

const std::array<uint8_t, ThorQ::Crypto::Signing::PublicKeyLen> ThorQ::Crypto::Signing::RootPk()
{
    // This is the root public key to verify key changes commited by the server
    // >>>>>>>>>>> DO NOT REMOVE OR MODIFY <<<<<<<<<<<
    return std::array<uint8_t, ThorQ::Crypto::Signing::PublicKeyLen> {
        0x55, 0x95, 0xA2, 0x25, 0x61, 0xCA, 0x29, 0xC8,
        0xF9, 0x19, 0x28, 0x7F, 0x22, 0x1C, 0xC4, 0x86,
        0x4F, 0x90, 0xBB, 0x1F, 0xAC, 0xDA, 0x00, 0x8A,
        0xFB, 0xDA, 0x31, 0xB0, 0x38, 0xC4, 0x8E, 0x51
    };
}

ThorQ::Crypto::Signing::Signing()
    : m_state()
    , m_modlock()
    , m_pk{0}
    , m_sk{0}
{
    assert(sodium_init() >= 0);
}

ThorQ::Crypto::Signing::~Signing()
{
    // Zero out memory to not leave a footprint in RAM
    reset();
}

void ThorQ::Crypto::Signing::reset()
{
    std::unique_lock l(m_modlock);
    reset_nolock();
}

bool ThorQ::Crypto::Signing::trySaveToFile(const char* cpath, bool onlyPublicKey) const
{
    std::filesystem::path path(cpath);

    // Buffer to write to
    std::vector<std::uint8_t> data;
    data.reserve(MaxFileSize);

    std::shared_lock l(const_cast<std::shared_mutex&>(m_modlock));

    // Insert public key
    data.insert(data.begin(), m_pk.begin(), m_pk.end());

    // Insert secret key
    if (!onlyPublicKey) {
        data.insert(data.end(), m_sk.begin(), m_sk.end());
    }

    std::size_t keysSize = data.size();
    data.resize(keysSize + ThorQ::Crypto::Signing::SignatureLen);

    std::uint8_t* keysPtr = data.data();
    std::uint8_t* signaturePtr = keysPtr + keysSize;

    // Create signature of the keypair thats being stored
    if (crypto_sign_ed25519_detached(signaturePtr, nullptr, keysPtr, keysSize, m_sk.data()) != 0) {
        return false;
    }

    // Write to file
    if (!tryWriteAll(path, data)) {
        return false;
    }

    return true;
}

bool ThorQ::Crypto::Signing::tryLoadFromFile(const char* cpath)
{
    std::filesystem::path path(cpath);

    // Dont try to read something that isnt a regular file
    if (!std::filesystem::is_regular_file(path)) {
        return false;
    }

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

    std::size_t keysSize = data.size() - Signing::SignatureLen;
    std::uint8_t* keysPtr = data.data();
    std::uint8_t* signaturePtr = data.data() + keysSize;

    // Check that the file signature is correct (signature, data, datasize, publicKey)
    if (crypto_sign_verify_detached(signaturePtr, keysPtr, keysSize, keysPtr) != 0) {
        return false;
    }

    // Write the data to this
    std::unique_lock l(m_modlock);
    memcpy(m_pk.data(), data.data(), Signing::PublicKeyLen);

    if (keysSize == Signing::PublicKeyLen) {
        m_state = State::OnlyPublicKey;
    }
    else {
        m_state = State::BothKeys;
        memcpy(m_sk.data(), data.data() + Signing::PublicKeyLen, Signing::SecretKeyLen);
    }

    return true;
}

bool ThorQ::Crypto::Signing::generateKeyPair()
{
    std::unique_lock l(m_modlock);
    if (crypto_sign_keypair(m_pk.data(), m_sk.data()) != 0) {
        return false;
    }
    m_state = State::BothKeys;
    return true;
}

bool ThorQ::Crypto::Signing::getPublicKey(std::span<uint8_t, ThorQ::Crypto::Signing::PublicKeyLen> publicKeyOut) const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(m_modlock));
    if (m_state != State::Uninitialized) {
        std::copy(m_pk.begin(), m_pk.end(), publicKeyOut.begin());
        return true;
    }
    return false;
}

bool ThorQ::Crypto::Signing::sign(const std::span<const uint8_t> data, std::span<uint8_t, ThorQ::Crypto::Signing::SignatureLen> signatureOut) const
{
    if (crypto_sign_detached(signatureOut.data(), nullptr, data.data(), data.size(), m_sk.data()) != 0) {
        return false;
    }
    return true;
}

bool ThorQ::Crypto::Signing::verify(const std::span<const uint8_t> data, const std::span<const uint8_t, ThorQ::Crypto::Signing::SignatureLen> signatureIn) const
{
    if (crypto_sign_verify_detached(signatureIn.data(), data.data(), data.size(), m_pk.data()) != 0) {
        return false;
    }
    return true;
}

void ThorQ::Crypto::Signing::reset_nolock()
{
    memset(m_pk.data(), 0, ThorQ::Crypto::Signing::PublicKeyLen);
    memset(m_sk.data(), 0, ThorQ::Crypto::Signing::SecretKeyLen);
    m_state = State::Uninitialized;
}
