#ifndef CRYPTO_H
#define CRYPTO_H

#include <span>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <shared_mutex>

#include <sodium.h>

namespace ThorQ {
/// Class to make cryptography extremely easy to deal with
class Crypto
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
    static void RandomizeBytes(std::uint8_t* data, std::size_t len);

    Crypto();
    ~Crypto();

    void reset();

    bool ready() const;

    /**
     * @brief Clear shared secret, and generate a new key pair
     * @return
     */
    bool generateKeyPair();

    /** Get the public key
     * @param publicKeyOut
     * @param outLen
     * @retval Returns if public key was successfully retrieved
     */
    bool getPublicKey(std::span<std::uint8_t> publicKeyOut) const;

    /** Establish secret key with foreign host
     * @param foreignKey
     * @param keySize
     * @return
     */
    bool agree(const std::span<std::uint8_t> foreignKey);

    /** Attempts to encrypt the data
     * @param inputData
     * @param outputData
     * @param dataLen
     * @param iv
     * @return
     */
    bool encrypt(std::span<std::uint8_t> dataOut, const std::span<std::uint8_t> dataIn, std::span<std::uint8_t> mac, std::span<std::uint8_t> nonce) const;

    /** Attempts to decrypt the data
     * @param inputData
     * @param outputData
     * @param dataLen
     * @param iv
     * @return
     */
    bool decrypt(std::span<std::uint8_t> dataOut, const std::span<std::uint8_t> dataIn, std::span<std::uint8_t> mac, std::span<std::uint8_t> nonce) const;
private:
    void reset_nolock();

    enum class State : std::uint8_t
    {
        Uninitialized,
    };

    std::atomic<State> m_state;
    std::shared_mutex  m_modlock;
    std::array<std::uint8_t, Crypto::PublicKeyLen>  m_pk;
    std::array<std::uint8_t, Crypto::SecretKeyLen>  m_sk;
    std::array<std::uint8_t, Crypto::SessionKeyLen> m_rx;
    std::array<std::uint8_t, Crypto::SessionKeyLen> m_tx;
};
}

#endif // CRYPTO_H
