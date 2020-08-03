#ifndef CRYPTO_H
#define CRYPTO_H

#include <vector>
#include <memory>
#include <cstdint>

namespace Botan {
class StreamCipher;
class AutoSeeded_RNG;
class Private_Key;
}

namespace ThorQ {
/** Class to make cryptography extremely easy to deal with
  */
class Crypto
{
	bool m_ready;
	Botan::AutoSeeded_RNG* m_rng;
    Botan::Private_Key* m_key;
	std::unique_ptr<Botan::StreamCipher> m_streamCipher;

    Crypto(Botan::Private_Key* key);
public:
    /** Randomizes data using cryptographic functions
     * @param data Pointer to data to randomize
     * @param len Length of data to randomize
     */
	static void RandomizeBytes(std::uint8_t* data, std::size_t len);

    /** Loads a cryptographic key pair, from disk (requires a password)
      * @param keyName Name of the file of the key pair (without the file extension)
      * @param password Passord to decrypt the key pair
      * @retval Returns a cryptoclass containing the key pair upon success, returns nullptr upon failure
      */
    static Crypto* load(const std::string& keyName, const std::string& password);

    /** Saves a cryptographic key pair, to disk (requires a password)
      * @param keyName Name of the file of the key pair (without the file extension)
      * @param password Passord to encrypt the key pair
      * @retval Returns if saving the key pair was a success
      */
    bool save(const std::string& keyName, const std::string& password) const;

	Crypto();
	~Crypto();

    /** Get the public key
     * @returns The generated public key
     */
    std::vector<std::uint8_t> publicKey() const;

    /** Checks if shared secret has been established
     * @returns if shared secret is established
     */
    bool ready() const;

    /** Establish secret key with foreign friend
     * @param data public key of foreign friend
     * @returns if key agreement succeeded
     */
    bool agree(const std::vector<std::uint8_t>& data);

    /** Clear shared secret, and generate a new key pair
     */
    void reset();

    /** Attempts to encrypt a vector as a reference
     * @param data Data to encrypt, this data will grow in size by a few bytes as a result of adding a IV to the end of it
     * @returns if the encryption was successful or not
     */
    bool encrypt(std::vector<std::uint8_t>& data);

    /** Attempts to encrypt a vector as a reference
     * @param data Data to encrypt
     * @param iv Data to write IV to
     * @returns if the encryption was successful or not
     */
    bool encrypt(std::vector<std::uint8_t>& data, std::uint8_t* iv);

    /** Attempts to decrypt a vector as a reference
     * @param data Data to decrypt, this data will shrink in size by a few bytes as a result of removing the IV from the end of it
     * @returns if the decryption was successful or not
     */
    bool decrypt(std::vector<std::uint8_t>& data);

    /** Attempts to decrypt a vector as a reference
     * @param data Data to decrypt
     * @param iv Data to read IV from
     * @returns if the decryption was successful or not
     */
    bool decrypt(std::vector<std::uint8_t>& data, const std::uint8_t* iv);
};
}

#endif // CRYPTO_H
