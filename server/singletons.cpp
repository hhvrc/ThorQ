#include "singletons.h"

#include <cryptography/signer.h>

std::shared_ptr<ThorQ::Crypto::Signer> g_signer = nullptr;

bool ThorQ::Singletons::Init()
{
    g_signer = std::make_shared<ThorQ::Crypto::Signer>();
    return g_signer->tryLoadFromFile("root_signer.pksk");
}

std::shared_ptr<ThorQ::Crypto::Signer> ThorQ::Singletons::rootSigner()
{
    return g_signer;
}
