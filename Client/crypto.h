#ifndef CRYPTO_H
#define CRYPTO_H

#include <vector>
#include <memory>
#include <cstdint>

namespace Botan {
	class StreamCipher;
	class ECDH_PrivateKey;
}

namespace ThorQ {
	class Crypto
	{
		bool m_ready;
		Botan::ECDH_PrivateKey* m_key;
		std::unique_ptr<Botan::StreamCipher> m_streamCipher;
	public:
		Crypto();
		~Crypto();
		std::vector<std::uint8_t> PublicKey() const;
		bool IsCryptoReady();
		bool Agree(std::vector<std::uint8_t> data);
		bool Agree(std::uint8_t* data, std::size_t len);
		std::vector<std::uint8_t> Encrypt(std::vector<std::uint8_t> data);
		std::vector<std::uint8_t> Encrypt(const std::uint8_t* data, std::size_t len);
		std::vector<std::uint8_t> Decrypt(std::vector<std::uint8_t> data);
		std::vector<std::uint8_t> Decrypt(const std::uint8_t* data, std::size_t len);
	};
}

#endif // CRYPTO_H
