#ifndef SIGNING_H
#define SIGNING_H

#include <sodium.h>

#include <filesystem>
#include <shared_mutex>
#include <span>
#include <array>
#include <string>
#include <atomic>
#include <cstdlib>
#include <cstdint>

namespace ThorQ {
namespace Crypto {
class Signer final
{
public:
    static constexpr std::size_t SignatureLen = crypto_sign_BYTES;
    static constexpr std::size_t PublicKeyLen = crypto_sign_PUBLICKEYBYTES;
    static constexpr std::size_t SecretKeyLen = crypto_sign_SECRETKEYBYTES;

    /**
     * @brief One signer to rule them all, this one is here to verify signer updates
     * @return The signer
     */
    static const std::array<std::uint8_t, Signer::PublicKeyLen> RootSigner();

    Signer();
    ~Signer();

    bool trySaveToFile(const char* path, bool onlyPublicKey) const;
    bool tryLoadFromFile(const char* path);

    /**
     * @brief Clear all instance data
     * @return
     */
    void clear();

    /**
     * @brief Clear shared secret, and generate a new key pair
     * @return
     */
    bool generateKeyPair();

    /**
     * @brief Getter for the public key
     * @return Returns a copy of the public key
     */
    inline std::array<std::uint8_t, Signer::PublicKeyLen> publicKey() const { return m_pk; }
    bool setPublicKey(const std::uint8_t* publicKey, std::size_t keySize);
    inline bool setPublicKey(const std::span<const std::uint8_t, Signer::PublicKeyLen> publicKey) { return setPublicKey(publicKey.data(), publicKey.size()); }

    /*
     *
     */
    bool sign(const std::uint8_t* data, std::size_t dataSize, std::uint8_t* signatureOut, std::size_t signatureSize) const;
    inline bool sign(std::span<const std::uint8_t> data, std::uint8_t* signatureOut, std::size_t signatureSize) const { return sign(data.data(), data.size(), signatureOut, signatureSize); }
    inline bool sign(const std::uint8_t* data, std::size_t dataSize, std::span<std::uint8_t> signatureOut) const { return sign(data, dataSize, signatureOut.data(), signatureOut.size()); }
    inline bool sign(std::span<const std::uint8_t> data, std::span<std::uint8_t> signatureOut) const { return sign(data.data(), data.size(), signatureOut.data(), signatureOut.size()); }

    bool verify(const std::uint8_t* data, std::size_t dataSize, const std::uint8_t* signature, std::size_t signatureSize) const;
    inline bool verify(std::span<const std::uint8_t> data, const std::uint8_t* signatureOut, std::size_t signatureSize) const { return verify(data.data(), data.size(), signatureOut, signatureSize); }
    inline bool verify(const std::uint8_t* data, std::size_t dataSize, std::span<const std::uint8_t> signatureOut) const { return verify(data, dataSize, signatureOut.data(), signatureOut.size()); }
    inline bool verify(std::span<const std::uint8_t> data, std::span<const std::uint8_t> signatureOut) const { return verify(data.data(), data.size(), signatureOut.data(), signatureOut.size()); }
private:
    std::array<std::uint8_t, Signer::PublicKeyLen>  m_pk;
    std::array<std::uint8_t, Signer::SecretKeyLen>  m_sk;

    enum class State : std::uint8_t
    {
        Uninitialized,
        OnlyPublicKey,
        BothKeys
    } m_state;
};
}
}

#endif // SIGNING_H
