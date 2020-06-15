#include "crypto.h"

#include <botan/hex.h>
#include <botan/ecdh.h>
#include <botan/chacha.h>
#include <botan/pubkey.h>
#include <botan/base64.h>
#include <botan/bcrypt.h>
#include <botan/system_rng.h>
#include <botan/stream_cipher.h>

using namespace ThorQ;

Crypto::Crypto() :
	m_ready(false),
	m_streamCipher(Botan::StreamCipher::create("ChaCha(20)"))
{
	m_key = new Botan::ECDH_PrivateKey(Botan::system_rng(), Botan::EC_Group("secp256r1"));
}

Crypto::~Crypto()
{
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

bool Crypto::Agree(std::vector<std::uint8_t> data)
{
	if (data.size() == m_key->public_value().size())
	{
		try
		{
			Botan::PK_Key_Agreement ecdh(*m_key, Botan::system_rng(), "KDF2(SHA-256)");
			m_streamCipher->set_key(ecdh.derive_key(32, data.data(), data.size()));
			m_ready = true;
			return true;
		}
		catch (std::exception ex)
		{
			fprintf(stderr, "Error while doing key agreement: %s\n", ex.what());
			fflush(stderr);
		}
	}

	return false;
}

bool Crypto::Agree(uint8_t* data, std::size_t len)
{
	if (len == m_key->public_value().size())
	{
		try
		{
			Botan::PK_Key_Agreement ecdh(*m_key, Botan::system_rng(), "KDF2(SHA-256)");
			m_streamCipher->set_key(ecdh.derive_key(32, data, len));
			m_ready = true;
			return true;
		}
		catch (std::exception ex)
		{
			fprintf(stderr, "Error while doing key agreement: %s\n", ex.what());
			fflush(stderr);
		}
	}

	return false;
}

std::vector<uint8_t> Crypto::Encrypt(std::vector<uint8_t> data)
{
	if (!data.empty())
	{
		try
		{
			std::vector<std::uint8_t> output(24 + data.size());

			Botan::system_rng().randomize(output.data(), 24);
			m_streamCipher->set_iv(output.data(), 24);

			m_streamCipher->encrypt(data);

			memcpy(output.data() + 24, data.data(), data.size());

			return output;
		}
		catch (std::exception ex)
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

			Botan::system_rng().randomize(output.data(), 24);
			m_streamCipher->set_iv(output.data(), 24);

			std::vector<std::uint8_t> vec(data, data + len);

			m_streamCipher->encrypt(vec);

			memcpy(output.data() + 24, vec.data(), vec.size());

			return output;
		}
		catch (std::exception ex)
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
		catch (std::exception ex)
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
		catch (std::exception ex)
		{
			fprintf(stderr, "Error while doing decryption: %s\n", ex.what());
			fflush(stderr);
		}
	}

	return std::vector<std::uint8_t>();
}
