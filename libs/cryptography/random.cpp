#include "random.h"

#include <sodium.h>

#include <cassert>

void ThorQ::Crypto::RandomizeBytes(std::span<uint8_t> bytes)
{
    assert(sodium_init() >= 0);
    randombytes_buf(bytes.data(), bytes.size());
}
