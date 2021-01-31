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

    /**
     * @brief Clear all instance data
     * @return
     */
    void reset();

    bool trySaveToFile(const char* path, bool onlyPublicKey) const;
    bool tryLoadFromFile(const char* path);

    /**
     * @brief Clear shared secret, and generate a new key pair
     * @return
     */
    bool generateKeyPair();

    /**
     * @brief Get the public key
     * @param publicKeyOut Span to write publicKey to
     * @retval Returns if public key was successfully retrieved
     */
    bool getPublicKey(std::span<std::uint8_t, Signer::PublicKeyLen> publicKeyOut) const;
    bool setPublicKey(const std::span<const std::uint8_t, Signer::PublicKeyLen> publicKeyIn);

    /*
     *
     */
    bool sign(const std::span<const std::uint8_t> data, std::span<std::uint8_t, Signer::SignatureLen> signatureOut) const;

    bool verify(const std::span<const std::uint8_t> data, const std::span<const std::uint8_t, Signer::SignatureLen> signatureIn) const;
private:
    void reset_nolock();

    enum class State : std::uint8_t
    {
        Uninitialized,
        OnlyPublicKey,
        BothKeys
    };

    std::atomic<State> m_state;
    std::shared_mutex  m_modlock;
    std::array<std::uint8_t, Signer::PublicKeyLen>  m_pk;
    std::array<std::uint8_t, Signer::SecretKeyLen>  m_sk;
};
}
}

#endif // SIGNING_H
