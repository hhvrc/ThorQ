#include "instance.h"

#include <iostream>

#include <botan/hex.h>
#include <botan/chacha.h>
#include <botan/pubkey.h>
#include <botan/base64.h>
#include <botan/bcrypt.h>
#include <botan/system_rng.h>
#include <botan/stream_cipher.h>

Instance::Instance() :
	m_name(""),
	m_ready(false),
	m_key(Botan::system_rng(), Botan::EC_Group("secp256r1")),
	m_streamCipher(Botan::StreamCipher::create("ChaCha(20)"))
{
}

Instance::Instance(std::string name) :
	m_name(name),
	m_ready(false),
	m_key(Botan::system_rng(), Botan::EC_Group("secp256r1")),
	m_streamCipher(Botan::StreamCipher::create("ChaCha(20)"))
{
}

Instance::~Instance()
{
}

void Instance::SetName(const std::string& newName)
{
	m_name = newName;
}

const std::string& Instance::Name() const
{
	return m_name;
}

std::vector<std::uint8_t> Instance::PublicKey() const
{
	return m_key.public_value();
}

bool Instance::IsCryptoReady()
{
	return m_ready;
}

bool Instance::Agree(std::uint8_t* data, std::size_t len)
{
	if (len == m_key.public_value().size())
	{
		try
		{
			Botan::PK_Key_Agreement ecdh(m_key, Botan::system_rng(), "KDF2(SHA-256)");
			m_streamCipher->set_key(ecdh.derive_key(32, data, len));
			m_ready = true;
			return true;
		}
		catch (std::exception ex)
		{
			std::cerr << ex.what() << std::endl;
		}
	}

	return false;
}

std::vector<std::uint8_t> Instance::Encrypt(std::vector<std::uint8_t> data)
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
			std::cerr << ex.what() << std::endl;
		}
	}

	return std::vector<std::uint8_t>();
}

std::vector<std::uint8_t> Instance::Encrypt(const std::uint8_t* data, std::size_t len)
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
			std::cerr << ex.what() << std::endl;
		}
	}

	return std::vector<std::uint8_t>();
}

std::vector<std::uint8_t> Instance::Decrypt(std::vector<std::uint8_t> data)
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
			std::cerr << ex.what() << std::endl;
		}
	}

	return std::vector<std::uint8_t>();
}

std::vector<std::uint8_t> Instance::Decrypt(const std::uint8_t* data, std::size_t len)
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
			std::cerr << ex.what() << std::endl;
		}
	}

	return std::vector<std::uint8_t>();
}
