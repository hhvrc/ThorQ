#include <cryptography/random.h>
#include <cryptography/signing.h>
#include <cryptography/encryption.h>
#include <utils.h>

#include <fmt/core.h>

#include <array>
#include <vector>
#include <cstring>
#include <memory>

int main(int argc, char** argv)
{
    THORQ_UNUSED(argc)
    THORQ_UNUSED(argv)

    ThorQ::Crypto::Signing signing;

    if (!signing.generateKeyPair()) {
        fmt::print(stderr, "Failed to generate keypair\n");
    }

    if (!signing.trySaveToFile("generated.pk", true) || !signing.trySaveToFile("generated.pksk", false)) {
        fmt::print(stderr, "Failed to save files\n");
    }

    fmt::print("Done\n");
    return EXIT_SUCCESS;
}
