#include "crypto.h"

#include <botan_all.h>
#include "constants.h"

using namespace ThorQ;

void Crypto::RandomizeBytes(std::uint8_t* data, std::size_t len)
{
	if (data == nullptr || len == 0)
		return;

	Botan::AutoSeeded_RNG().randomize(data, len);
}

Crypto::Crypto()
    : m_ready(false)
    , m_rng(new Botan::AutoSeeded_RNG())
    , m_key(new Botan::ECDH_PrivateKey(*m_rng, Botan::EC_Group(CRYPTO_EC_OID_NAME)))
    , m_streamCipher(Botan::StreamCipher::create(CRYPTO_CIPHER_NAME))
{
}

Crypto::~Crypto()
{
    delete m_rng;
	delete m_key;
}

std::vector<uint8_t> Crypto::PublicKey() const
{
	return m_key->public_value();
}

bool Crypto::IsCryptoReady()
{
	return m_ready;
}

bool Crypto::Agree(const std::vector<uint8_t>& data)
{
    if (data.size() == m_key->public_value().size())
	{
		try
        {
            Botan::PK_Key_Agreement ecdh(*m_key, *m_rng, CRYPTO_KEY_DERIVATION_FUNCTION);
            m_streamCipher->set_key(ecdh.derive_key(CRYPTO_KEY_LENGTH, data));
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

void Crypto::Reset()
{
	try
	{
		m_ready = false;
		Botan::ECDH_PrivateKey* oldKey = m_key;
        m_key = new Botan::ECDH_PrivateKey(*m_rng, Botan::EC_Group(CRYPTO_EC_OID_NAME));
		delete oldKey;
		m_streamCipher->clear();
	}
	catch (Botan::Exception ex)
	{
		fprintf(stderr, "Error while resetting encryption: %s\n", ex.what());
		fflush(stderr);
	}
}

bool Crypto::Encrypt(std::vector<uint8_t>& data)
{
	if (!data.empty())
    {
		try
        {
            std::uint8_t iv[CRYPTO_IV_LENGTH];
            m_rng->randomize(iv, CRYPTO_IV_LENGTH);
            m_streamCipher->set_iv(iv, CRYPTO_IV_LENGTH);

            m_streamCipher->encrypt(data);

			data.reserve(data.size() + CRYPTO_IV_LENGTH);

			data.insert(data.end(), iv, iv + CRYPTO_IV_LENGTH);
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
bool Crypto::Decrypt(std::vector<uint8_t>& data)
{
    if (data.size() > CRYPTO_IV_LENGTH)
	{
		try
        {
			std::size_t newSize = data.size() - CRYPTO_IV_LENGTH;

			m_streamCipher->set_iv(data.data() + newSize, CRYPTO_IV_LENGTH);

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
