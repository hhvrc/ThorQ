#include "crypto.h"

#include <fstream>
#include <QDebug>

#include <botan_all.h>

#include "constants.h"

#include <openssl/evp.h>

bool afsa(std::uint8_t* key, std::uint8_t* iv, const std::vector<std::uint8_t>& data, std::vector<std::uint8_t>& out)
{
    out.resize(data.size());

    int outputLenght, tempLength;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    EVP_EncryptInit_ex(ctx, EVP_chacha20(), nullptr, key, iv);

    if (!EVP_EncryptUpdate(ctx, out.data(), &outputLenght, data.data(), data.size()))
    {
        EVP_CIPHER_CTX_reset(ctx);
        return false;
    }

    if (!EVP_EncryptFinal_ex(ctx, out.data() + outputLenght, &tempLength))
    {
         EVP_CIPHER_CTX_reset(ctx);
         return false;
     }

     outputLenght += tempLength;

     EVP_CIPHER_CTX_reset(ctx);

     return true;
}


using namespace ThorQ;

Crypto::Crypto(Botan::Private_Key *key)
    : m_ready(false)
    , m_rng(new Botan::AutoSeeded_RNG())
    , m_key(key)
    , m_streamCipher(Botan::StreamCipher::create(THORQ_CRYPTO_CIPHER_NAME))
    , m_cipher(EVP_chacha20())
    , m_ctx(EVP_CIPHER_CTX_new())
{
}

void Crypto::RandomizeBytes(std::uint8_t* data, std::size_t len)
{
    if (data == nullptr || len == 0)
        return;

    Botan::AutoSeeded_RNG().randomize(data, len);
}

Crypto* Crypto::load(const std::string &keyName, const std::string &password)
{
    try
    {
        std::unique_ptr<Botan::AutoSeeded_RNG> rng = std::make_unique<Botan::AutoSeeded_RNG>();

        Botan::Private_Key* pk = Botan::PKCS8::load_key(keyName + ".sk", *rng, password);

        return new Crypto(pk);
    }
    catch (const Botan::Exception& ex)
    {
        qDebug() << "Error while decoding key:" << ex.what();
    }
    catch (const std::exception& ex)
    {
        qDebug() << "Error while loading key:" << ex.what();
    }

    return nullptr;
}

bool Crypto::save(const std::string &keyName, const std::string &password) const
{
    try
    {
        std::string encoded = Botan::PKCS8::PEM_encode(*m_key, *m_rng, password);
        std::ofstream out(keyName + ".sk");
        out << encoded;
        out.close();
    }
    catch (const Botan::Exception& ex)
    {
        qDebug() << "Error while encoding key:" << ex.what();
        return false;
    }
    catch (const std::exception& ex)
    {
        qDebug() << "Error while saving key:" << ex.what();
        return false;
    }

    return true;
}

Crypto::Crypto()
    : m_ready(false)
    , m_rng(new Botan::AutoSeeded_RNG())
    , m_key(new Botan::ECDH_PrivateKey(*m_rng, Botan::EC_Group(THORQ_CRYPTO_EC_ID)))
    , m_streamCipher(Botan::StreamCipher::create(THORQ_CRYPTO_CIPHER_NAME))
    , m_cipher(EVP_chacha20())
    , m_ctx(EVP_CIPHER_CTX_new())
{
}

Crypto::~Crypto()
{
    delete m_rng;
    delete m_key;
    EVP_CIPHER_CTX_free(m_ctx);
}

std::vector<std::uint8_t> Crypto::publicKey() const
{
    return m_key->public_key_bits();
}

bool Crypto::ready() const
{
    return m_ready;
}

bool Crypto::agree(const std::vector<std::uint8_t>& data)
{
    if (data.size() == m_key->public_key_bits().size())
    {
        try
        {
            Botan::PK_Key_Agreement ecdh(*m_key, *m_rng, THORQ_CRYPTO_KEY_DVFUNC);
            m_streamCipher->set_key(ecdh.derive_key(THORQ_CRYPTO_KEY_LENGTH, data));
            m_ready = true;
            return true;
        }
        catch (const std::exception& ex)
        {
            qDebug() << "Error while doing key agreement:" << ex.what();
        }
    }

    return false;
}

void Crypto::reset()
{
    try
    {
        m_ready = false;
        Botan::Private_Key* oldKey = m_key;
        m_key = new Botan::ECDH_PrivateKey(*m_rng, Botan::EC_Group(THORQ_CRYPTO_EC_ID));
        delete oldKey;
        m_streamCipher->clear();
    }
    catch (const std::exception& ex)
    {
        qDebug() << "Error while resetting encryption:" << ex.what();
    }
}

bool Crypto::encrypt(std::vector<std::uint8_t>& data)
{
    if (!data.empty())
    {
        try
        {
            data.reserve(data.size() + THORQ_CRYPTO_CIPHER_IV_LEN);

            std::uint8_t iv[THORQ_CRYPTO_CIPHER_IV_LEN];
            m_rng->randomize(iv, THORQ_CRYPTO_CIPHER_IV_LEN);
            m_streamCipher->set_iv(iv, THORQ_CRYPTO_CIPHER_IV_LEN);

            m_streamCipher->encrypt(data);

            data.insert(data.end(), iv, &iv[THORQ_CRYPTO_CIPHER_IV_LEN]);

            return true;
        }
        catch (const std::exception& ex)
        {
            qDebug() << "Error while doing encryption:" << ex.what();
        }
    }
    return false;
}

bool Crypto::encrypt(std::vector<std::uint8_t> &data, std::uint8_t* iv)
{
    if (!data.empty())
    {
        try
        {
            m_rng->randomize(iv, THORQ_CRYPTO_CIPHER_IV_LEN);
            m_streamCipher->set_iv(iv, THORQ_CRYPTO_CIPHER_IV_LEN);

            m_streamCipher->encrypt(data);
            return true;
        }
        catch (const std::exception& ex)
        {
            qDebug() << "Error while doing encryption:" << ex.what();
        }
    }
    return false;
}

bool Crypto::decrypt(std::vector<std::uint8_t>& data)
{
    if (data.size() > THORQ_CRYPTO_CIPHER_IV_LEN)
    {
        try
        {
            std::size_t newSize = data.size() - THORQ_CRYPTO_CIPHER_IV_LEN;

            m_streamCipher->set_iv(&data[newSize], THORQ_CRYPTO_CIPHER_IV_LEN);

            data.resize(newSize);

            m_streamCipher->decrypt(data);
            return true;
        }
        catch (const std::exception& ex)
        {
            qDebug() << "Error while doing decryption:" << ex.what();
        }
    }
    return false;
}

bool Crypto::decrypt(std::vector<std::uint8_t> &data, const std::uint8_t *iv)
{
    if (!data.empty())
    {
        try
        {
            m_streamCipher->set_iv(iv, THORQ_CRYPTO_CIPHER_IV_LEN);

            m_streamCipher->decrypt(data);
            return true;
        }
        catch (const std::exception& ex)
        {
            qDebug() << "Error while doing decryption:" << ex.what();
        }
    }
    return false;
}
