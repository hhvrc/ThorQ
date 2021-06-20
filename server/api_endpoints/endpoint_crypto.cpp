#include "endpoint_crypto.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <cryptography/signer.h>
#include <schemas_common.h>
#include <flatbuffers/stl_emulation.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::CryptoEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto fbsClientKey = context.body<ThorQ::Serialization::Crypto::Message>()->body_as_client_key();
    auto connection = context.apiConnection();

    if (fbsClientKey == nullptr) {
        fmt::print(stderr, "[CRYPTO] Got nullptr!\n");
        connection->disconnect();
        return;
    }

    auto& crypto = connection->crypto();

    if (!crypto.generateKeyPair()) {
        fmt::print(stderr, "[CRYPTO] Failed to generate keypair!\n");
        return;
    }

    const auto& clientPublicKey = *fbsClientKey->public_key();

    // Set client key
    crypto.setForeignKey(fromFbsArray<std::uint8_t, 32>(clientPublicKey)); // TODO: clientPublicKey.DataSpan()

    auto serverPublicKey = crypto.publicKey();

    ThorQ::Crypto::Signer signer;
    signer.generateKeyPair();

    // TODO: load this at server startup
    if (!signer.tryLoadFromFile("root_signing.pksk")) {
        fmt::print(stderr, "[CRYPTO] Failed to load signer keypair!\n");
        return;
    }

    // PUBLIC_CLIENT_KEY + PUBLIC_SERVER_KEY
    std::array<std::uint8_t, ThorQ::Crypto::Signer::PublicKeyLen * 2> combinedPublicKeys;

    auto pk1 = combinedPublicKeys.data();
    auto pk2 = pk1 + ThorQ::Crypto::Encryption::PublicKeyLen;

    std::memcpy(pk1, clientPublicKey.Data(), ThorQ::Crypto::Encryption::PublicKeyLen);
    std::memcpy(pk2, serverPublicKey.data(), ThorQ::Crypto::Encryption::PublicKeyLen);

    // SIGNATURE(PUBLIC_CLIENT_KEY + PUBLIC_SERVER_KEY)
    std::array<std::uint8_t, ThorQ::Crypto::Signer::SignatureLen> signature;
    if (!signer.sign(combinedPublicKeys, signature)) {
        fmt::print(stderr, "[CRYPTO] Failed to sign encryption public key!\n");
        return;
    }

    // Build flatbuffer
    auto fbsServerKey = context.fbsBuilder().CreateStruct(ThorQ::Serialization::Crypto::ServerKey(serverPublicKey, signature)).Union();
    auto fbsCrypto    = ThorQ::Serialization::Crypto::CreateMessage(context.fbsBuilder(), ThorQ::Serialization::Crypto::Body_server_key, fbsServerKey).Union();
    context.addMessage(ThorQ::Serialization::Body_crypto, fbsCrypto);

    if (context.sendDataUnencrypted()) {
        fmt::print("[CONNECTION] Crypto Established\n");
    }
}
