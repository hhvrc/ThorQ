#ifndef CRYPTO_H
#define CRYPTO_H

#include <vector>
#include <memory>
#include <cstdint>

namespace Botan {
	class StreamCipher;
    class AutoSeeded_RNG;
    class ECDH_PrivateKey;
}

namespace ThorQ {
	class Crypto
	{
		bool m_ready;
        Botan::AutoSeeded_RNG* m_rng;
		Botan::ECDH_PrivateKey* m_key;
		std::unique_ptr<Botan::StreamCipher> m_streamCipher;
    public:
        static std::vector<std::uint8_t> GenRandBytes(std::size_t len);

		Crypto();
		~Crypto();
		std::vector<std::uint8_t> PublicKey() const;
		bool IsCryptoReady();
		bool Agree(std::vector<std::uint8_t> data);
		bool Agree(std::uint8_t* data, std::size_t len);
		void Reset();
		std::vector<std::uint8_t> Encrypt(std::vector<std::uint8_t> data);
		std::vector<std::uint8_t> Encrypt(const std::uint8_t* data, std::size_t len);
		std::vector<std::uint8_t> Decrypt(std::vector<std::uint8_t> data);
        std::vector<std::uint8_t> Decrypt(const std::uint8_t* data, std::size_t len);
	};
}

#endif // CRYPTO_H
