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
/** Class to make cryptography extremely easy to deal with
  */
class Crypto
{
	bool m_ready;
	Botan::AutoSeeded_RNG* m_rng;
	Botan::ECDH_PrivateKey* m_key;
	std::unique_ptr<Botan::StreamCipher> m_streamCipher;
public:
    /** Randomizes data using cryptographic functions
     * @param data Pointer to data to randomize
     * @param len Length of data to randomize
     */
	static void RandomizeBytes(std::uint8_t* data, std::size_t len);

	Crypto();
	~Crypto();

    /** Get the public key
     * @returns The generated public key
     */
	std::vector<std::uint8_t> PublicKey() const;

    /** Checks if shared secret has been established
     * @returns if shared secret is established
     */
    bool ready();

    /** Establish secret key with foreign friend
     * @param data public key of foreign friend
     * @returns if key agreement succeeded
     */
	bool Agree(const std::vector<std::uint8_t>& data);

    /** Clear shared secret, and generate a new key pair
     */
	void Reset();

    /** Attempts to encrypt a vector as a reference
     * @param data Data to encrypt, this data will grow in size by a few bytes as a result of adding a IV to the end of it
     * @returns if the encryption was successful or not
     */
	bool Encrypt(std::vector<std::uint8_t>& data);

    /** Attempts to decrypt a vector as a reference
     * @param data Data to decrypt, this data will shrink in size by a few bytes as a result of removing the IV from the end of it
     * @returns if the decryption was successful or not
     */
	bool Decrypt(std::vector<std::uint8_t>& data);
};
}

#endif // CRYPTO_H
