#ifndef CRYPTO_H
#define CRYPTO_H

#include <sodium.h>

#include <span>
#include <array>
#include <cstdlib>
#include <cstdint>

namespace ThorQ {
namespace Crypto {
/// Class to make cryptography extremely easy to deal with
class Encryption final
{
public:
    static constexpr std::size_t MacLen = crypto_box_MACBYTES;
    static constexpr std::size_t NonceLen = crypto_box_NONCEBYTES;
    static constexpr std::size_t DataOverhead = MacLen + NonceLen;
    static constexpr std::size_t PublicKeyLen = crypto_box_PUBLICKEYBYTES;
    static constexpr std::size_t SecretKeyLen = crypto_box_SECRETKEYBYTES;

    Encryption();
    ~Encryption();

    bool trySaveToFile(const char* path, bool onlyPublicKey) const;
    bool tryLoadFromFile(const char* path);

    void clear();
    void clearForgeinKey();

    bool generateKeyPair();

    inline std::array<std::uint8_t, Encryption::PublicKeyLen> publicKey() const { return m_pk; }
    inline std::array<std::uint8_t, Encryption::PublicKeyLen> foreignKey() const { return m_fk; }

    bool setForeignKey(const std::uint8_t* publicKey, std::size_t keySize);
    inline bool setForeignKey(const std::span<const std::uint8_t, Encryption::PublicKeyLen> publicKey) { return setForeignKey(publicKey.data(), publicKey.size()); }

    bool encrypt(const std::uint8_t* dataIn, std::size_t dataInSize, std::uint8_t* dataOut, std::size_t dataOutSize) const;
    inline bool encrypt(std::span<const std::uint8_t> dataIn, std::uint8_t* dataOut, std::size_t dataOutSize) const { return encrypt(dataIn.data(), dataIn.size(), dataOut, dataOutSize); }
    inline bool encrypt(const std::uint8_t* dataIn, std::size_t dataInSize, std::span<std::uint8_t> dataOut) const { return encrypt(dataIn, dataInSize, dataOut.data(), dataOut.size()); }
    inline bool encrypt(std::span<const std::uint8_t> dataIn, std::span<std::uint8_t> dataOut) const { return encrypt(dataIn.data(), dataIn.size(), dataOut.data(), dataOut.size()); }

    bool decrypt(const std::uint8_t* dataIn, std::size_t dataInSize, std::uint8_t* dataOut, std::size_t dataOutSize) const;
    inline bool decrypt(std::span<const std::uint8_t> dataIn, std::uint8_t* dataOut, std::size_t dataOutSize) const { return decrypt(dataIn.data(), dataIn.size(), dataOut, dataOutSize); }
    inline bool decrypt(const std::uint8_t* dataIn, std::size_t dataInSize, std::span<std::uint8_t> dataOut) const { return decrypt(dataIn, dataInSize, dataOut.data(), dataOut.size()); }
    inline bool decrypt(std::span<const std::uint8_t> dataIn, std::span<std::uint8_t> dataOut) const { return decrypt(dataIn.data(), dataIn.size(), dataOut.data(), dataOut.size()); }
private:
    std::array<std::uint8_t, Encryption::PublicKeyLen> m_pk;
    std::array<std::uint8_t, Encryption::SecretKeyLen> m_sk;
    std::array<std::uint8_t, Encryption::PublicKeyLen> m_fk;
};
}
}

#endif // CRYPTO_H
