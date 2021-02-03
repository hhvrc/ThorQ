#include <cryptography/encryption.h>

#include <fmt/core.h>

int main()
{
    ThorQ::Crypto::Encryption signer;

    if (!signer.generateKeyPair()) {
        fmt::print(stderr, "Failed to generate keypair\n");
    }

    if (!signer.trySaveToFile("generated.pk", true) || !signer.trySaveToFile("generated.pksk", false)) {
        fmt::print(stderr, "Failed to save files\n");
    }

    fmt::print("Done\n");
    return EXIT_SUCCESS;
}
