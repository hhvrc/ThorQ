#include <cryptography/signer.h>
#include <cryptography/encryption.h>
#include <cryptography/passwordhash.h>

#include <fmt/core.h>

#include <vector>
#include <cstring>
#include <cstdint>

bool testEncryption(std::size_t testDataSize, std::size_t iterations)
{
    ThorQ::Crypto::Encryption client;
    ThorQ::Crypto::Encryption server;

    if (!client.generateKeyPair()) {
        fmt::print("Encryption keygen failed!\n");
        return false;
    }
    if (!client.trySaveToFile("keys.pksk", false)) {
        fmt::print("Encryption trySaveToFile failed!\n");
        return false;
    }
    if (!server.tryLoadFromFile("keys.pksk")) {
        fmt::print("Encryption tryLoadFromFile failed!\n");
        return false;
    }
    if (server.publicKey() != client.publicKey()) {
        fmt::print("Encryption loaded file invalid!\n");
        return false;
    }

    std::vector<std::uint8_t> org, enc, dec;
    org.resize(testDataSize);
    enc.resize(testDataSize + ThorQ::Crypto::Encryption::DataOverhead);
    dec.resize(testDataSize);

    for (std::size_t i = 0; i < iterations; i++)
    {
        // Reset all data
        client.clear();
        server.clear();

        memset(org.data(), 0, org.size());
        memset(enc.data(), 0, enc.size());
        memset(dec.data(), 0, dec.size());

        // Generate keys
        if (!client.generateKeyPair() || !server.generateKeyPair())
        {
            fmt::print("Encryption keygen failed!\n");
            return false;
        }

        auto clientPk = client.publicKey();
        auto serverPk = server.publicKey();

        // Exchange the public keys
        if (!client.setForeignKey(serverPk.data(), serverPk.size()) ||
            !server.setForeignKey(clientPk.data(), clientPk.size()))
        {
            fmt::print("Encryption agree failed!\n");
            return false;
        }

        // Randomize input data
        randombytes_buf(org.data(), org.size());

        // Encrypt the data
        if (!client.encrypt(org.data(), org.size(), enc.data(), enc.size()))
        {
            fmt::print("Encryption encrypt failed!\n");
            return false;
        }

        // Check if encryption changed the data
        if (memcmp(org.data(), enc.data(), org.size()) == 0)
        {
            fmt::print("Encryption did nothing!\n");
            return false;
        }

        // Decrypt the encrpyted data using the other crypto object
        if (!server.decrypt(enc.data(), enc.size(), dec.data(), dec.size()))
        {
            fmt::print("Encryption decrypt failed!\n");
            return false;
        }

        // Check if original data and decrypted data is the same
        if (memcmp(org.data(), dec.data(), org.size()) != 0)
        {
            fmt::print("Encryption result invalid!\n");
            return false;
        }
    }

    return true;
}

bool testSigning()
{
    ThorQ::Crypto::Signer signer;

    // Generate keys
    if (!signer.generateKeyPair())
    {
        fmt::print("Signing keygen failed!\n");
        return false;
    }

    // Test saving only the public key
    if (!signer.trySaveToFile("test.pk", true))
    {
        fmt::print("Signing savekey failed!\n");
        return false;
    }

    // Save the keys
    if (!signer.trySaveToFile("test.pksk", false))
    {
        fmt::print("Signing savekey(s) failed!\n");
        return false;
    }

    signer.clear();

    // Test loading only a single key
    if (!signer.tryLoadFromFile("test.pk"))
    {
        fmt::print("Signing loadkey failed!\n");
        return false;
    }

    signer.clear();

    // Load the keys
    if (!signer.tryLoadFromFile("test.pksk"))
    {
        fmt::print("Signing loadkey(s) failed!\n");
        return false;
    }

    std::uint8_t testData[2048];
    randombytes_buf(testData, sizeof(testData));

    std::array<std::uint8_t, ThorQ::Crypto::Signer::SignatureLen> signature;

    if (!signer.sign(testData, signature))
    {
        fmt::print("Signing data failed!\n");
        return false;
    }

    if (!signer.verify(testData, signature))
    {
        fmt::print("Signing data failed!\n");
        return false;
    }

    return true;
}

bool testPasswordHashing()
{
    std::string password = "Very secure password";

    std::array<std::uint8_t, ThorQ::Crypto::PasswordHash::HashLength> key;

    if (!ThorQ::Crypto::PasswordHash::Generate(password.c_str(), password.length(), key)) {
        fmt::print("PasswordHashing generate failed!\n");
        return false;
    }

    if (!ThorQ::Crypto::PasswordHash::Verify(password.c_str(), password.length(), key)) {
        fmt::print("PasswordHashing verify failed!\n");
        return false;
    }

    return true;
}

int main()
{
    if (!testEncryption(2048, 128)) {
        return EXIT_FAILURE;
    }

    if (!testSigning()) {
        return EXIT_FAILURE;
    }

    if (!testPasswordHashing()) {
        return EXIT_FAILURE;
    }

    fmt::print("Success!\n");
    exit(EXIT_SUCCESS);
}
