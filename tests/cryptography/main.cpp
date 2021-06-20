#include <cryptography/signer.h>
#include <cryptography/encryption.h>
#include <cryptography/hashing.h>

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
    if (std::memcmp(server.publicKey().data(), client.publicKey().data(), ThorQ::Crypto::Encryption::PublicKeyLen) != 0) {
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

        std::memset(org.data(), 0, org.size());
        std::memset(enc.data(), 0, enc.size());
        std::memset(dec.data(), 0, dec.size());

        // Generate keys
        if (!client.generateKeyPair() || !server.generateKeyPair())
        {
            fmt::print("Encryption keygen failed!\n");
            return false;
        }

        client.setForeignKey(server.publicKey());
        server.setForeignKey(client.publicKey());

        // Randomize input data
        randombytes_buf(org.data(), org.size());

        // Encrypt the data
        if (!client.encrypt(org.data(), org.size(), enc.data(), enc.size()))
        {
            fmt::print("Encryption encrypt failed!\n");
            return false;
        }

        // Check if encryption changed the data
        if (std::memcmp(org.data(), enc.data(), org.size()) == 0)
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
        if (std::memcmp(org.data(), dec.data(), org.size()) != 0)
        {
            fmt::print("Encryption result invalid!\n");
            return false;
        }
    }

    return true;
}

bool testSigning()
{
    ThorQ::Crypto::Signer origSigner;

    // Generate keys
    if (!origSigner.generateKeyPair())
    {
        fmt::print("Signing keygen failed!\n");
        return false;
    }

    // Test saving only the public key
    if (!origSigner.trySaveToFile("test.pk", true))
    {
        fmt::print("Signing savekey failed!\n");
        return false;
    }

    // Save both public and secret key
    if (!origSigner.trySaveToFile("test.pksk", false))
    {
        fmt::print("Signing savekey(s) failed!\n");
        return false;
    }

    // Test loading only the public key
    ThorQ::Crypto::Signer pkSigner;
    if (!pkSigner.tryLoadFromFile("test.pk"))
    {
        fmt::print("Signing loadkey failed!\n");
        return false;
    }

    // Load both public and secret key
    ThorQ::Crypto::Signer pkskSigner;
    if (!pkskSigner.tryLoadFromFile("test.pksk"))
    {
        fmt::print("Signing loadkey(s) failed!\n");
        return false;
    }

    std::uint8_t testData[2048];
    randombytes_buf(testData, sizeof(testData));

    std::array<std::uint8_t, ThorQ::Crypto::Signer::SignatureLen> signature;

    if (!origSigner.sign(testData, signature))
    {
        fmt::print("Signing data failed!\n");
        return false;
    }

    if (!pkskSigner.verify(testData, signature))
    {
        fmt::print("Verifying data failed!\n");
        return false;
    }

    if (!pkSigner.verify(testData, signature))
    {
        fmt::print("Verifying data failed!\n");
        return false;
    }

    return true;
}

bool testPasswordHashing()
{
    std::string password = "Very secure password";

    ThorQ::Crypto::Hashing::Hash hash;
    ThorQ::Crypto::Hashing::Salt salt;
    ThorQ::Crypto::Hashing::Parameters parameters;

    ThorQ::Crypto::Hashing::generateSalt(salt);
    parameters.setPerformance(ThorQ::Crypto::Hashing::Parameters::Performance::Interactive);
    if (!ThorQ::Crypto::Hashing::Generate(password, salt, parameters, hash)) {
        fmt::print("PasswordHashing generate failed!\n");
        return false;
    }

    ThorQ::Crypto::Hashing::generateSalt(salt);
    parameters.setPerformance(ThorQ::Crypto::Hashing::Parameters::Performance::Moderate);
    if (!ThorQ::Crypto::Hashing::Generate(password, salt, parameters, hash)) {
        fmt::print("PasswordHashing generate failed!\n");
        return false;
    }

    ThorQ::Crypto::Hashing::generateSalt(salt);
    parameters.setPerformance(ThorQ::Crypto::Hashing::Parameters::Performance::Sensitive);
    if (!ThorQ::Crypto::Hashing::Generate(password, salt, parameters, hash)) {
        fmt::print("PasswordHashing generate failed!\n");
        return false;
    }

    return true;
}

int main()
{
    fmt::print("Testing encryption...\n");
    if (!testEncryption(2048, 128)) {
        return EXIT_FAILURE;
    }

    fmt::print("Testing signing...\n");
    if (!testSigning()) {
        return EXIT_FAILURE;
    }

    fmt::print("Testing password hashing...\n");
    if (!testPasswordHashing()) {
        return EXIT_FAILURE;
    }

    fmt::print("Success!\n");
    exit(EXIT_SUCCESS);
}
