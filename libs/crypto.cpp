#include "crypto.h"

#include <fstream>

#include <log.h>
#include <botan_all.h>
#include "constants.h"


using namespace ThorQ;

Crypto::Crypto(Botan::Private_Key *key)
    : m_ready(false)
    , m_rng(new Botan::AutoSeeded_RNG())
    , m_key(key)
    , m_streamCipher(Botan::StreamCipher::create(THORQ_CRYPTO_CIPHER_NAME))
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
        Botan::AutoSeeded_RNG rng = Botan::AutoSeeded_RNG();

        Botan::Private_Key* pk = Botan::PKCS8::load_key(keyName + ".sk", rng, password);

        return new Crypto(pk);
    }
    catch (Botan::Exception ex)
    {
        thorq_error("Error while decoding key: %s\n", ex.what())
    }
    catch (std::exception ex)
    {
        thorq_error("Error while loading key: %s\n", ex.what())
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
    catch (Botan::Exception ex)
    {
        thorq_error("Error while encoding key: %s\n", ex.what())
        return false;
    }
    catch (std::exception ex)
    {
        thorq_error("Error while saving key: %s\n", ex.what())
        return false;
    }

    return true;
}

Crypto::Crypto()
	: m_ready(false)
	, m_rng(new Botan::AutoSeeded_RNG())
	, m_key(new Botan::ECDH_PrivateKey(*m_rng, Botan::EC_Group(THORQ_CRYPTO_EC_ID)))
	, m_streamCipher(Botan::StreamCipher::create(THORQ_CRYPTO_CIPHER_NAME))
{
}

Crypto::~Crypto()
{
	delete m_rng;
	delete m_key;
}

std::vector<std::uint8_t> Crypto::publicKey() const
{
	return m_key->public_value();
}

bool Crypto::ready() const
{
	return m_ready;
}

bool Crypto::agree(const std::vector<std::uint8_t>& data)
{
	if (data.size() == m_key->public_value().size())
	{
		try
		{
			Botan::PK_Key_Agreement ecdh(*m_key, *m_rng, THORQ_CRYPTO_KEY_DVFUNC);
			m_streamCipher->set_key(ecdh.derive_key(THORQ_CRYPTO_KEY_LENGTH, data));
			m_ready = true;
			return true;
		}
		catch (Botan::Exception ex)
		{
			fprintf(stderr, "Error while doing key agreement: %s\n", ex.what());
			fflush(stderr);
		}
	}

	return false;
}

void Crypto::reset()
{
	try
	{
		m_ready = false;
		Botan::ECDH_PrivateKey* oldKey = m_key;
		m_key = new Botan::ECDH_PrivateKey(*m_rng, Botan::EC_Group(THORQ_CRYPTO_EC_ID));
		delete oldKey;
		m_streamCipher->clear();
	}
	catch (Botan::Exception ex)
	{
		fprintf(stderr, "Error while resetting encryption: %s\n", ex.what());
		fflush(stderr);
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

			data.insert(data.end(), iv, iv + THORQ_CRYPTO_CIPHER_IV_LEN);

			return true;
		}
		catch (Botan::Exception ex)
		{
			fprintf(stderr, "Error while doing encryption: %s\n", ex.what());
			fflush(stderr);
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
        catch (Botan::Exception ex)
        {
            fprintf(stderr, "Error while doing encryption: %s\n", ex.what());
            fflush(stderr);
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

			m_streamCipher->set_iv(data.data() + newSize, THORQ_CRYPTO_CIPHER_IV_LEN);

			data.resize(newSize);

			m_streamCipher->decrypt(data);
			return true;
		}
		catch (Botan::Exception ex)
		{
			fprintf(stderr, "Error while doing decryption: %s\n", ex.what());
			fflush(stderr);
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
        catch (Botan::Exception ex)
        {
            fprintf(stderr, "Error while doing decryption: %s\n", ex.what());
            fflush(stderr);
        }
    }
    return false;
}
