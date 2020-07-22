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
/**
  * @brief The Crypto class
  */
class Crypto
{
	bool m_ready;
	Botan::AutoSeeded_RNG* m_rng;
	Botan::ECDH_PrivateKey* m_key;
	std::unique_ptr<Botan::StreamCipher> m_streamCipher;
public:
	static void RandomizeBytes(std::uint8_t* data, std::size_t len);

	Crypto();
	~Crypto();
	std::vector<std::uint8_t> PublicKey() const;
	bool IsCryptoReady();
	bool Agree(const std::vector<std::uint8_t>& data);
	void Reset();
	bool Encrypt(std::vector<std::uint8_t>& data);
	bool Decrypt(std::vector<std::uint8_t>& data);
};
}

#endif // CRYPTO_H
