#include "hashing.h"

#include <openssl/evp.h>
#include "botan_all.h"

bool ThorQ::Security::Hashing::Sha256_Hash(std::uint8_t *data, std::size_t size, std::uint8_t *hashOut)
{
    int ret = 0;
    unsigned int outSize = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == nullptr)
    {
        return false;
    }

    EVP_MD_CTX_set_flags(ctx, EVP_MD_CTX_FLAG_ONESHOT);

    ret = EVP_DigestInit(ctx, EVP_sha256());
    if (ret == 0)
    {
        goto err;
    }

    ret = EVP_DigestUpdate(ctx, data, size);
    if (ret == 0)
    {
        goto err;
    }

    ret = EVP_DigestFinal(ctx, hashOut, &outSize);
    if (ret == 0)
    {
        goto err;
    }

    ret = 1;
err:
    EVP_MD_CTX_free(ctx);
    return ret == 1;
}

std::string ThorQ::Security::Hashing::BCrypt_Hash(const std::string& password)
{
    Botan::AutoSeeded_RNG rng = Botan::AutoSeeded_RNG();
    return Botan::generate_bcrypt(password, rng);
}

bool ThorQ::Security::Hashing::BCrypt_Verify(const std::string& password, const std::string& hash)
{
    return Botan::check_bcrypt(password, hash);
}
