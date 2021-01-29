#include "apiconnectionhandler.h"

#include <networking/message.h>
#include <networking/abstractconnection.h>
#include <systemid.h>
#include <encoding.h>
#include <enums.h>

#include <schemas/account_generated.h>
#include <schemas/announcement_generated.h>
#include <schemas/device_generated.h>
#include <schemas/crypto_generated.h>
#include <schemas/file_generated.h>
#include <schemas/version_generated.h>
#include <schemas/systemid_generated.h>
#include <schemas/message_generated.h>
#include <schemas/group_generated.h>
#include <schemas/moderation_generated.h>

#include <fmt/core.h>

#include <mutex>

ThorQ::ApiConnectionHandler::ApiConnectionHandler()
    : m_crypto()
    , l_buffer()
    , m_buffer(THORQ_PAYLOAD_LEN_MAX)
{
    fmt::print("[CONNECTION] Constructed\n");
}

ThorQ::ApiConnectionHandler::~ApiConnectionHandler()
{
    fmt::print("[CONNECTION] Destroyed\n");
}

void ThorQ::ApiConnectionHandler::onConnect()
{
    fmt::print("[CONNECTION] Connected\n");
}

void ThorQ::ApiConnectionHandler::onDisconnect()
{
    fmt::print("[CONNECTION] Disconnected\n");
}

bool ThorQ::ApiConnectionHandler::onHeader(std::shared_ptr<ThorQ::Networking::MessageHeader> header)
{
    fmt::print("[CONNECTION] Header\n");
    if (header->size > ThorQ::Encoding::calculateEncodedSize(THORQ_PAYLOAD_LEN_MAX, true) || header->size < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }
    return true;
}

void ThorQ::ApiConnectionHandler::onMessage(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (!ThorQ::Encoding::validateEncodedData(*message)) {
        fmt::print(stderr, "Message is invalid!!!\n");
    }

    fmt::print("[CONNECTION] Message\n");

    std::scoped_lock l(l_buffer);
    m_buffer.resize(ThorQ::Encoding::calculateDecodedSize(*message));
    if (!ThorQ::Encoding::dataDecode(*message, m_buffer, m_crypto))
    {
        fmt::print(stderr, "Cannot unpack/decrypt packet\n");
        return;
    }

    auto fbsMessage = flatbuffers::GetRoot<ThorQ::Serialization::Message>(m_buffer.data());
    auto fbsVerifier = flatbuffers::Verifier(m_buffer.data(), m_buffer.size());

    if (!fbsMessage->Verify(fbsVerifier))
    {
        fmt::print(stderr, "Invalid flatbuffer message\n");
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        fmt::print("Invalid flatbuffer message\n");
        fmt::print("[MSG] account\n");
        //handleMessageAccount(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_crypto:
        handleMessageCrypto(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_system_id:
        fmt::print("[MSG] systemid\n");
        //handleMessageSystemID(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_group:
        fmt::print("[MSG] group\n");
        //handleMessageGroup(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_device:
        fmt::print("[MSG] device\n");
        //handleMessageCollar(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_moderation:
        fmt::print("[MSG] moderation\n");
        //handleMessageModeration(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_friend_request:
        fmt::print("[MSG] friend request\n");
        //handleMessageFriendRequest(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_file:
        fmt::print("[MSG] file\n");
        //handleMessageFile(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_user:
        fmt::print("[MSG] user\n");
        //handleMessageUser(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_announcement:
        fmt::print("[MSG] announcement\n");
        //handleMessageAnnouncement(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_NONE:
        fmt::print("[MSG] none\n");
    default:
        return;
    }
}

void ThorQ::ApiConnectionHandler::requestCrypto()
{
    fmt::print("Request Crypto\n");

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsRequest = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Request).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsRequest);
    fbsBuilder.Finish(fbsMessage);

    encodeAndSend(fbsBuilder.GetBufferSpan(), false);
}

void ThorQ::ApiConnectionHandler::handleMessageVersion(const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsVersion = reinterpret_cast<const ThorQ::Serialization::Version*>(body);

    if (!fbsVersion->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] version\n");

    ThorQ::Version version;
    version.setMajor(fbsVersion->major());
    version.setMinor(fbsVersion->minor());
    version.setPatch(fbsVersion->patch());

    switch ((THORQ_APP)fbsVersion->app()) {
    case THORQ_APP::SERVER:
        if (version == ThorQ::ServerVersion) {
            fmt::print("[VERSION] Server version matched\n");
        } else {
            fmt::print("[VERSION] Server version mismatched\n");
        }

        requestCrypto(); // TODO: this is bad
        break;
    case THORQ_APP::CLIENT:
        if (version == ThorQ::ClientVersion) {
            fmt::print("[VERSION] Client version matched\n");
        } else {
            fmt::print("[VERSION] Client version mismatched\n");
        }
        break;
    case THORQ_APP::LINK:
        if (version == ThorQ::LinkVersion) {
            fmt::print("[VERSION] Link version matched\n");
        } else {
            fmt::print("[VERSION] Link version mismatched\n");
        }
        break;
    default:
        fmt::print("[VERSION] invalid\n");
        return;
    }
}

void ThorQ::ApiConnectionHandler::handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(body);

    if (!fbsCrypto->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] crypto\n");

    switch (fbsCrypto->type()) {
    case ThorQ::Serialization::Crypto::MessageType_Establish:
    {
        fmt::print("[MSG] crypto establish!\n");

        if (fbsCrypto->data()->size() != ThorQ::Crypto::PublicKeyLen)
        {
            fmt::print("Got key with invalid length!\n");
            return;
        }

        m_crypto.generateKeyPair();

        std::span<std::uint8_t, ThorQ::Crypto::PublicKeyLen> data(
                        const_cast<std::uint8_t*>(fbsCrypto->data()->data()),
                        fbsCrypto->data()->size()
                    );

        if (m_crypto.agreeAsClient(data))
        {
            m_crypto.getPublicKey(data);

            // Build flatbuffer
            flatbuffers::FlatBufferBuilder fbsBuilder;
            auto fbsEstablish = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Establish, fbsBuilder.CreateVector(data.data(), data.size())).Union();
            auto fbsMessage   = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsEstablish);
            fbsBuilder.Finish(fbsMessage);

            encodeAndSend(fbsBuilder.GetBufferSpan(), false);
        }
        else
        {
            fmt::print(stderr, "Failed to create shared secret\n");
            connection()->disconnect(); // TODO: In the future should find a way to send reason for disconnet to server as well
        }
        break;
    }
    case ThorQ::Serialization::Crypto::MessageType_Verify:
    {
        fmt::print("[MSG] crypto verify! {}\n", fbsCrypto->data()->size());;

        if (fbsCrypto->data()->size() == THORQ_CRYPTO_VERIFICATION_DATA_LEN)
        {
            fmt::print("[MSG] crypto verified!\n");

            // Build flatbuffer
            flatbuffers::FlatBufferBuilder fbsBuilder;
            auto fbsVerify  = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Verify, fbsBuilder.CreateVector(fbsCrypto->data()->data(), fbsCrypto->data()->size())).Union();
            auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsVerify);
            fbsBuilder.Finish(fbsMessage);

            encodeAndSend(fbsBuilder.GetBufferSpan(), true);
        }
        else
        {
            fmt::print(stderr, "Failed to verify\n");
            connection()->disconnect(); // TODO: In the future should find a way to send reason for disconnet to server as well
        }
        break;
    }
    default:
        fmt::print("[MSG] crypto \?\?\?!\n\n");
        return;
    }
}

void ThorQ::ApiConnectionHandler::encodeAndSend(flatbuffers::span<std::uint8_t> buffer, bool encrypt)
{
    auto message = std::make_shared<std::vector<std::uint8_t>>();
    message->resize(ThorQ::Encoding::calculateEncodedSize(buffer.size(), encrypt));

    if (encrypt) {
        if (!ThorQ::Encoding::dataEncode(std::span<std::uint8_t>(buffer.data(), buffer.size()), *message, m_crypto)) {
            fmt::print(stderr, "Failed to encrypt message\n");
            return;
        }
    }
    else {
        if (!ThorQ::Encoding::dataEncode(std::span<std::uint8_t>(buffer.data(), buffer.size()), *message)) {
            fmt::print(stderr, "Failed to encode message\n");
            return;
        }
    }

    connection()->send(message);
}
