#include "hashing.h"

#include <thread>

#include <botan_all.h>

std::string HashingFunc(std::string password)
{
    try
    {
        Botan::AutoSeeded_RNG rng = Botan::AutoSeeded_RNG();
        return Botan::generate_bcrypt(password, rng);
    }
    catch (...) {}

    return {};
}

bool VerificationFunc(std::string password, std::string hash)
{
    return Botan::check_bcrypt(password, hash);
}

std::future<std::string> ThorQ::Security::Hashing::Hash(const std::string &password)
{
    return std::async(HashingFunc, password);
}

std::future<bool> ThorQ::Security::Hashing::Verify(const std::string &password, const std::string& hash)
{
    return std::async(VerificationFunc, password, hash);
}
