#include <cryptography/random.h>
#include <cryptography/signing.h>
#include <cryptography/encryption.h>
#include <utils.h>

#include <fmt/core.h>

#include <array>
#include <vector>
#include <cstring>
#include <memory>

bool testEncryption(std::size_t testDataSize, std::size_t iterations)
{
    ThorQ::Crypto::Encryption client;
    ThorQ::Crypto::Encryption server;

    std::vector<std::uint8_t> org, enc, dec;
    org.resize(testDataSize);
    enc.resize(testDataSize);
    dec.resize(testDataSize);

    std::array<std::uint8_t, ThorQ::Crypto::Encryption::MacLen> mac;
    std::array<std::uint8_t, ThorQ::Crypto::Encryption::NonceLen> nonce;
    std::array<std::uint8_t, ThorQ::Crypto::Encryption::PublicKeyLen> clientKey, serverKey;

    for (std::size_t i = 0; i < iterations; i++)
    {
        // Reset all data
        client.reset();
        server.reset();
        memset(org.data(), 0, org.size());
        memset(enc.data(), 0, enc.size());
        memset(dec.data(), 0, dec.size());
        memset(mac.data(), 0, mac.size());
        memset(nonce.data(), 0, nonce.size());

        // Generate keys
        if (!client.generateKeyPair() || !server.generateKeyPair())
        {
            fmt::print("Encryption keygen failed!\n");
            return false;
        }

        // Get the public keys
        if (!client.getPublicKey(clientKey) || !server.getPublicKey(serverKey))
        {
            fmt::print("Encryption getkey failed!\n");
            return false;
        }

        // Exchange the public keys
        if (!client.agreeAsClient(serverKey) || !server.agreeAsServer(clientKey))
        {
            fmt::print("Encryption agree failed!\n");
            return false;
        }

        // Randomize input data
        ThorQ::Crypto::RandomizeBytes(org);

        // Randomize nonce
        ThorQ::Crypto::RandomizeBytes(nonce);

        // Encrypt the data
        if (!client.encrypt(enc, org, mac, nonce))
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
        if (!server.decrypt(dec, enc, mac, nonce))
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
    ThorQ::Crypto::Signing signer;

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

    signer.reset();

    // Test loading only a single key
    if (!signer.tryLoadFromFile("test.pk"))
    {
        fmt::print("Signing loadkey failed!\n");
        return false;
    }

    signer.reset();

    // Load the keys
    if (!signer.tryLoadFromFile("test.pksk"))
    {
        fmt::print("Signing loadkey(s) failed!\n");
        return false;
    }

    std::array<std::uint8_t, 2048> testData;
    ThorQ::Crypto::RandomizeBytes(testData);

    std::array<std::uint8_t, ThorQ::Crypto::Signing::SignatureLen> signature;

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

int main(int argc, char** argv)
{
    THORQ_UNUSED(argc)
    THORQ_UNUSED(argv)

    if (!testEncryption(2048, 128)) {
        return EXIT_FAILURE;
    }

    if (!testSigning()) {
        return EXIT_FAILURE;
    }

    fmt::print("Success!\n");
    exit(EXIT_SUCCESS);
}
