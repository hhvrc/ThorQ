#ifndef CRYPTO_H
#define CRYPTO_H

#include <sodium.h>

#include <shared_mutex>
#include <span>
#include <array>
#include <atomic>
#include <cstdlib>
#include <cstdint>

namespace ThorQ {
namespace Crypto {
/// Class to make cryptography extremely easy to deal with
class Encryption final
{
public:
    static constexpr std::size_t MacLen = crypto_secretbox_MACBYTES;
    static constexpr std::size_t NonceLen = crypto_secretbox_NONCEBYTES;
    static constexpr std::size_t PublicKeyLen = crypto_kx_PUBLICKEYBYTES;
    static constexpr std::size_t SecretKeyLen = crypto_kx_SECRETKEYBYTES;
    static constexpr std::size_t SessionKeyLen = crypto_kx_SESSIONKEYBYTES;

    /** Randomizes data using cryptographic functions
     * @param data Pointer to data to randomize
     * @param len Length of data to randomize
     */
    static void RandomizeBytes(std::span<std::uint8_t> bytes);

    Encryption();
    ~Encryption();

    /**
     * @brief Clear all instance data
     * @return
     */
    void reset();

    /**
     * @brief Checks if instance is ready to encrypt data
     * @return
     */
    bool ready() const;

    /**
     * @brief Clear shared secret, and generate a new key pair
     * @return
     */
    bool generateKeyPair();

    /** Get the public key
     * @param publicKeyOut Span to write publicKey to
     * @retval Returns if public key was successfully retrieved
     */
    bool getPublicKey(std::span<std::uint8_t, Encryption::PublicKeyLen> publicKeyOut) const;

    /** Establish secret key with foreign host
     * @param foreignKey foreign public key to agree with
     * @return Returns if shared secret was computed
     */
    bool agreeAsServer(const std::span<const std::uint8_t, Encryption::PublicKeyLen> foreignKey);

    /** Establish secret key with foreign host
     * @param foreignKey foreign public key to agree with
     * @return Returns if shared secret was computed
     */
    bool agreeAsClient(const std::span<const std::uint8_t, Encryption::PublicKeyLen> foreignKey);

    /** Attempts to encrypt the data
     * @param dataOut span to write encrypted data to, this is the same size as dataIn
     * @param outputData span which contains cleartext to encrypt
     * @param mac message authentication code, this is a size of MacLen
     * @param nonce randomized data to make message unique, this is a size of NonceLen (encrypt function will randomize this)
     * @return
     */
    bool encrypt(std::span<std::uint8_t> dataOut, const std::span<const std::uint8_t> dataIn, std::span<std::uint8_t, Encryption::MacLen> mac, std::span<std::uint8_t, Encryption::NonceLen> nonce) const;

    /** Attempts to decrypt the data
     * @param dataOut span to write cleartext data to, this is the same size as dataIn
     * @param outputData span which contains encrypted cleartext
     * @param mac message authentication code, this is a size of MacLen
     * @param nonce randomized data to make message unique, this is a size of NonceLen
     * @return
     */
    bool decrypt(std::span<std::uint8_t> dataOut, const std::span<const std::uint8_t> dataIn, const std::span<const std::uint8_t, Encryption::MacLen> mac, const std::span<const std::uint8_t, Encryption::NonceLen> nonce) const;
private:
    /**
     * @brief reset, but without locking the shared mutex
     */
    void reset_nolock();
    void reset_shared_nolock();

    enum class State : std::uint8_t
    {
        Uninitialized,
        GeneratedKeys,
        Ready
    };

    std::atomic<State> m_state;
    std::shared_mutex  m_modlock;
    std::array<std::uint8_t, Encryption::PublicKeyLen>  m_pk;
    std::array<std::uint8_t, Encryption::SecretKeyLen>  m_sk;
    std::array<std::uint8_t, Encryption::SessionKeyLen> m_rx;
    std::array<std::uint8_t, Encryption::SessionKeyLen> m_tx;
};
}
}

#endif // CRYPTO_H
