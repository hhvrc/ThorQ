#ifndef SINGLETONS_H
#define SINGLETONS_H

#include <typedefs_global.h>
#include <typedefs_server.h>

#include <memory>

namespace ThorQ {
namespace Singletons {
bool Init();
std::shared_ptr<ThorQ::Crypto::Signer> rootSigner();
}
}

#endif // SINGLETONS_H
