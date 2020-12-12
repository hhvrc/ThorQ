#include <array>
#include <cstring>
#include <memory>

#include <crypto.h>
#include <fmt/core.h>

int main(int argc, char** argv)
{
    ThorQ::Crypto client;
    ThorQ::Crypto server;

    std::array<std::uint8_t, ThorQ::Crypto::PublicKeyLen> clientKey, serverKey;

    std::array<std::uint8_t, 2048> org{0}, enc{0}, dec{0};
    std::array<std::uint8_t, ThorQ::Crypto::MacLen> mac;
    std::array<std::uint8_t, ThorQ::Crypto::NonceLen> nonce;

    for (int i = 0; i < 10000; i++)
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
            fmt::print("Crypto keygen failed!\n");
            exit(EXIT_FAILURE);
        }

        // Get the public keys
        if (!client.getPublicKey(clientKey) || !server.getPublicKey(serverKey))
        {
            fmt::print("Crypto getkey failed!\n");
            exit(EXIT_FAILURE);
        }

        // Exchange the public keys
        if (!client.agreeAsClient(serverKey) || !server.agreeAsServer(clientKey))
        {
            fmt::print("Crypto agree failed!\n");
            exit(EXIT_FAILURE);
        }

        // Randomize input data
        ThorQ::Crypto::RandomizeBytes(org);

        // Randomize nonce
        ThorQ::Crypto::RandomizeBytes(nonce);

        // Encrypt the data
        if (!client.encrypt(enc, org, mac, nonce))
        {
            fmt::print("Crypto encrypt failed!\n");
            exit(EXIT_FAILURE);
        }

        // Check if encryption changed the data
        if (memcmp(org.data(), enc.data(), org.size()) == 0)
        {
            fmt::print("Crypto did nothing!\n");
            exit(EXIT_FAILURE);
        }

        // Decrypt the encrpyted data using the other crypto object
        if (!server.decrypt(dec, enc, mac, nonce))
        {
            fmt::print("Crypto decrypt failed!\n");
            exit(EXIT_FAILURE);
        }

        // Check if original data and decrypted data is the same
        if (memcmp(org.data(), dec.data(), org.size()) != 0)
        {
            fmt::print("Crypto result invalid!\n");
            exit(EXIT_FAILURE);
        }
    }

    fmt::print("Success!\n");
    exit(EXIT_SUCCESS);
}
