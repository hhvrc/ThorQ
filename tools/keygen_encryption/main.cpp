#include <sodium.h>
#include <utils.h>

#include <fmt/core.h>

#include <fstream>
#include <iostream>
#include <array>

int main(int argc, char** argv)
{
    THORQ_UNUSED(argc)
    THORQ_UNUSED(argv)

    std::array<std::uint8_t, crypto_kx_PUBLICKEYBYTES> pk;
    std::array<std::uint8_t, crypto_kx_SECRETKEYBYTES> sk;

    if (crypto_kx_keypair(pk.data(), sk.data()) != 0) {
        fmt::print(stderr, "Failed to generate keypair\n");
        return EXIT_FAILURE;
    }

    std::fstream pkout("generated.pk", std::ios::out), skout("generated.sk", std::ios::out);

    if (!pkout.is_open() || !skout.is_open()) {
        fmt::print(stderr, "Failed to open files!\n");
        return EXIT_FAILURE;
    }

    pkout.write((char*)pk.data(), pk.size());
    skout.write((char*)pk.data(), pk.size());

    pkout.close();
    skout.close();

    fmt::print("Done\n");
    return EXIT_SUCCESS;
}
