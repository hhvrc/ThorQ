#include "crypto.h"

#include <botan_all.h>

using namespace ThorQ;

std::vector<uint8_t> Crypto::GenRandBytes(std::size_t len)
{
    if (len == 0)
        return std::vector<uint8_t>();

    std::vector<std::uint8_t> output(len);
    Botan::AutoSeeded_RNG().randomize(output.data(), len);

    return output;
}

Crypto::Crypto()
    : m_ready(false)
    , m_rng(new Botan::AutoSeeded_RNG())
    , m_key(new Botan::ECDH_PrivateKey(*m_rng, Botan::EC_Group("secp256r1")))
    , m_streamCipher(Botan::StreamCipher::create("ChaCha(20)"))
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

#include <iostream>
bool Crypto::Agree(std::vector<std::uint8_t> data)
{
	return Agree(data.data(), data.size());
}

bool Crypto::Agree(uint8_t* data, std::size_t len)
{
	if (len == m_key->public_value().size())
	{
		try
        {
            Botan::PK_Key_Agreement ecdh(*m_key, *m_rng, "KDF2(SHA-256)");
			m_streamCipher->set_key(ecdh.derive_key(32, data, len));
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
        m_key = new Botan::ECDH_PrivateKey(*m_rng, Botan::EC_Group("secp256r1"));
		delete oldKey;
		m_streamCipher->clear();
	}
	catch (Botan::Exception ex)
	{
		fprintf(stderr, "Error while resetting encryption: %s\n", ex.what());
		fflush(stderr);
	}
}

std::vector<uint8_t> Crypto::Encrypt(std::vector<uint8_t> data)
{
	if (!data.empty())
	{
		try
		{
			std::vector<std::uint8_t> output(24 + data.size());

            m_rng->randomize(output.data(), 24);
			m_streamCipher->set_iv(output.data(), 24);

			m_streamCipher->encrypt(data);

			memcpy(output.data() + 24, data.data(), data.size());

			return output;
		}
		catch (Botan::Exception ex)
		{
			fprintf(stderr, "Error while doing encryption: %s\n", ex.what());
			fflush(stderr);
		}
	}

	return std::vector<std::uint8_t>();
}

std::vector<uint8_t> Crypto::Encrypt(const uint8_t* data, std::size_t len)
{
	if (data != nullptr && len != 0)
	{
		try
		{
			std::vector<std::uint8_t> output(24 + len);

            m_rng->randomize(output.data(), 24);
			m_streamCipher->set_iv(output.data(), 24);

			std::vector<std::uint8_t> vec(data, data + len);

			m_streamCipher->encrypt(vec);

			memcpy(output.data() + 24, vec.data(), vec.size());

			return output;
		}
		catch (Botan::Exception ex)
		{
			fprintf(stderr, "Error while doing encryption: %s\n", ex.what());
			fflush(stderr);
		}
	}

	return std::vector<std::uint8_t>();
}

std::vector<uint8_t> Crypto::Decrypt(std::vector<uint8_t> data)
{
	if (data.size() > 24)
	{
		try
		{
			m_streamCipher->set_iv(data.data(), 24);

			std::vector<std::uint8_t> dataWithoutIv(data.begin() + 24, data.end());
			m_streamCipher->decrypt(dataWithoutIv);

			return dataWithoutIv;
		}
		catch (Botan::Exception ex)
		{
			fprintf(stderr, "Error while doing decryption: %s\n", ex.what());
			fflush(stderr);
		}
	}

	return std::vector<std::uint8_t>();
}

std::vector<uint8_t> Crypto::Decrypt(const uint8_t* data, std::size_t len)
{
	if (data != nullptr && len > 24)
	{
		try
		{
			m_streamCipher->set_iv(data, 24);

			std::vector<std::uint8_t> dataWithoutIv(data + 24, data + len);

			m_streamCipher->decrypt(dataWithoutIv);

			return dataWithoutIv;
		}
		catch (Botan::Exception ex)
		{
			fprintf(stderr, "Error while doing decryption: %s\n", ex.what());
			fflush(stderr);
		}
	}

	return std::vector<std::uint8_t>();
}
