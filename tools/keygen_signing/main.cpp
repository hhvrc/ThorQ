#include <cryptography/signer.h>
#include <utils.h>

#include <fmt/core.h>

int main(int argc, char** argv)
{
    THORQ_UNUSED(argc)
    THORQ_UNUSED(argv)

    ThorQ::Crypto::Signer signer;

    if (!signer.generateKeyPair()) {
        fmt::print(stderr, "Failed to generate keypair\n");
    }

    if (!signer.trySaveToFile("generated.pk", true) || !signer.trySaveToFile("generated.pksk", false)) {
        fmt::print(stderr, "Failed to save files\n");
    }

    fmt::print("Done\n");
    return EXIT_SUCCESS;
}
