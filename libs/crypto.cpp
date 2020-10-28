#include "crypto.h"

#include <limits>
#include <cstring>
#include <cstdint>
#include <algorithm>

#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>

// EC_ID:         NID_secp256k1
// CIPHER_ID:     ChaCha20
// KEY_DER_FN_ID: KDF2(SHA-256)

bool ThorQ::Crypto::RandomizeBytes(uint8_t *data, std::size_t len)
{
    return RAND_bytes(data, len) == 1;
}

ThorQ::Crypto::Crypto()
    : m_ctx(EVP_CIPHER_CTX_new())
    , m_group(nullptr)
    , m_cipher(EVP_chacha20())
    , m_keyPair(nullptr)
{
    memset(m_sharedKey, 0, CRYPTO_ECDH_SHARED_KEY_LEN);
}

ThorQ::Crypto::~Crypto()
{
    reset();

    EVP_CIPHER_CTX_free(m_ctx);
}

void ThorQ::Crypto::reset()
{
    memset(m_sharedKey, 0, CRYPTO_ECDH_SHARED_KEY_LEN);
    if (m_keyPair != nullptr)
    {
        EC_KEY_free(m_keyPair);
    }
    m_keyPair = nullptr;
    m_group = nullptr;
}

bool ThorQ::Crypto::ready() const
{
    return m_keyPair != nullptr && m_group != nullptr;
}

bool ThorQ::Crypto::generateKeyPair()
{
    reset();

    //Generate keypair
    m_keyPair = EC_KEY_new_by_curve_name(CRYPTO_CURVE_NID);

    // Get group
    m_group = EC_KEY_get0_group(m_keyPair);

    // Generate keys
    if (!EC_KEY_generate_key(m_keyPair))
    {
        reset();
        return false;
    }

    return true;
}

bool ThorQ::Crypto::getPublicKey(std::uint8_t* publicKeyOut, std::size_t outLen) const
{
    if (ready())
    {
        // Get public key
        const EC_POINT* publicKey = EC_KEY_get0_public_key(m_keyPair);

        // Encode public key
        std::size_t len = EC_POINT_point2oct(m_group,
                                 publicKey,
                                 POINT_CONVERSION_UNCOMPRESSED,
                                 publicKeyOut,
                                 outLen,
                                 nullptr);

        return len == CRYPTO_ECDH_PUBLIC_KEY_LEN;
    }

    return false;
}

bool ThorQ::Crypto::agree(const std::uint8_t* keyData, std::size_t keySize)
{
    bool success = false;

    if (ready())
    {
        // Create key
        EC_POINT* foreignKeyPoint = EC_POINT_new(m_group);

        if (foreignKeyPoint != nullptr)
        {
            // Decode foreign key
            if (EC_POINT_oct2point(m_group, foreignKeyPoint, keyData, keySize, nullptr) == 1)
            {
                // Calculate shared secret
                success = (ECDH_compute_key(m_sharedKey, CRYPTO_ECDH_SHARED_KEY_LEN, foreignKeyPoint, m_keyPair, nullptr) == CRYPTO_ECDH_SHARED_KEY_LEN);
            }

            EC_POINT_free(foreignKeyPoint);
        }
    }

    return success;
}

bool ThorQ::Crypto::encrypt(std::uint8_t* outputData, const std::uint8_t* inputData, std::size_t dataLen, std::uint8_t* iv)
{
    if (!ready()) return false;

    bool ret = false;
    int iterWrittenBytes = 0;
    std::size_t totalWrittenBytes = 0;

    if (RandomizeBytes(iv, CRYPTO_AES_IV_LEN))
    {
        // Initialize the cipher
        if (EVP_EncryptInit_ex(m_ctx, m_cipher, nullptr, m_sharedKey, iv) == 1)
        {
            // Iterate for every 2.27 ish GB (INT_MAX)
            while (totalWrittenBytes != dataLen)
            {
                // Prevent integer overflow
                int bytesToWrite = std::min((int)(dataLen - totalWrittenBytes), std::numeric_limits<int>::max());

                if (EVP_EncryptUpdate(m_ctx, outputData + totalWrittenBytes, &iterWrittenBytes, inputData + totalWrittenBytes, bytesToWrite) == 1)
                {
                    totalWrittenBytes += iterWrittenBytes;
                }
                else
                {
                    goto err;
                }
            }

            // Fianlize the encryption
            if (EVP_EncryptFinal_ex(m_ctx, outputData + totalWrittenBytes, &iterWrittenBytes) == 1)
            {
                totalWrittenBytes += iterWrittenBytes;

                // Check if everything got written
                ret = (totalWrittenBytes == dataLen);
            }
        }
    }
err:
    EVP_CIPHER_CTX_reset(m_ctx);
    return ret;
}

bool ThorQ::Crypto::decrypt(std::uint8_t* outputData, const std::uint8_t* inputData, std::size_t dataLen, const std::uint8_t *iv)
{
    if (!ready()) return false;

    bool ret = false;
    int iterWrittenBytes = 0;
    std::size_t totalWrittenBytes = 0;

    // Initialize the cipher
    if (EVP_DecryptInit_ex(m_ctx, m_cipher, nullptr, m_sharedKey, iv) == 1)
    {
        // Iterate for every 2.27 ish GB (INT_MAX)
        while (totalWrittenBytes != dataLen)
        {
            // Prevent integer overflow
            int bytesToWrite = std::min((int)(dataLen - totalWrittenBytes), std::numeric_limits<int>::max());

            if (EVP_DecryptUpdate(m_ctx, outputData + totalWrittenBytes, &iterWrittenBytes, inputData + totalWrittenBytes, bytesToWrite) == 1)
            {
                totalWrittenBytes += iterWrittenBytes;
            }
            else
            {
                goto err;
            }
        }

        // Fianlize the encryption
        if (EVP_DecryptFinal_ex(m_ctx, outputData + totalWrittenBytes, &iterWrittenBytes) == 1)
        {
            totalWrittenBytes += iterWrittenBytes;

            // Check if everything got written
            ret = (totalWrittenBytes == dataLen);
        }
    }
err:
    EVP_CIPHER_CTX_reset(m_ctx);
    return ret;
}
