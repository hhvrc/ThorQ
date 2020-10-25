#ifndef CRYPTO_H
#define CRYPTO_H

#include <cstdint>
#include <cstdlib>

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct ec_group_st EC_GROUP;
typedef struct evp_cipher_st EVP_CIPHER;
typedef struct ec_key_st EC_KEY;

#define CRYPTO_CURVE_NID NID_secp256k1
#define CRYPTO_AES_IV_LEN 12
#define CRYPTO_ECDH_SHARED_KEY_LEN 32
#define CRYPTO_ECDH_PUBLIC_KEY_LEN 65
#define CRYPTO_ECDH_PRIVATE_KEY_LEN 32

namespace ThorQ {
/// Class to make cryptography extremely easy to deal with
class Crypto
{
public:
    /** Randomizes data using cryptographic functions
     * @param data Pointer to data to randomize
     * @param len Length of data to randomize
     */
    static bool RandomizeBytes(std::uint8_t* data, std::size_t len);

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
    bool getPublicKey(std::uint8_t* publicKeyOut, std::size_t outLen) const;

    /** Establish secret key with foreign host
     * @param foreignKey
     * @param keySize
     * @return
     */
    bool agree(const std::uint8_t* foreignKey, std::size_t keySize);

    /** Attempts to encrypt the data
     * @param inputData
     * @param outputData
     * @param dataLen
     * @param iv
     * @return
     */
    bool encrypt(std::uint8_t* outputData, const std::uint8_t* inputData, std::size_t dataLen, std::uint8_t* iv);

    /** Attempts to decrypt the data
     * @param inputData
     * @param outputData
     * @param dataLen
     * @param iv
     * @return
     */
    bool decrypt(std::uint8_t* outputData, const std::uint8_t *inputData, std::size_t dataLen, const std::uint8_t *iv);
private:
    EVP_CIPHER_CTX* m_ctx;
    const EC_GROUP* m_group;
    const EVP_CIPHER* m_cipher;

    EC_KEY* m_keyPair;
    std::uint8_t m_sharedKey[CRYPTO_ECDH_SHARED_KEY_LEN];
};
}

#endif // CRYPTO_H
