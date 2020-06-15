#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace Botan {
class StreamCipher;
}

#include <botan/ecdh.h>

class Instance
{
	std::string m_name;
	bool m_ready;
	Botan::ECDH_PrivateKey m_key;
	std::unique_ptr<Botan::StreamCipher> m_streamCipher;

	Instance(const Instance&) = delete;
	Instance& operator=(const Instance&) = delete;
public:
	Instance();
	Instance(std::string name);
	~Instance();
	void SetName(const std::string& newName);
	const std::string& Name() const;
	std::vector<std::uint8_t> PublicKey() const;
	bool IsCryptoReady();
	bool Agree(std::uint8_t* data, std::size_t len);
	std::vector<std::uint8_t> Encrypt(std::vector<std::uint8_t> data);
	std::vector<std::uint8_t> Encrypt(const std::uint8_t* data, std::size_t len);
	std::vector<std::uint8_t> Decrypt(std::vector<std::uint8_t> data);
	std::vector<std::uint8_t> Decrypt(const std::uint8_t* data, std::size_t len);
};

#endif // INSTANCE_H
