#include "apiconnectionhandler.h"

#include <networking/message.h>
#include <systemid.h>
#include <encoding.h>
#include <crypto.h>

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

ThorQ::ApiConnectionHandler::ApiConnectionHandler()
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
    fmt::print("[CONNECTION] Message\n");
}

/*
#include <QDebug>

#include <constants.h>
#include <encoding.h>

#include <schemas/message_generated.h>
#include <schemas/friendrequest_generated.h>

#include "

ThorQ::ServerHandler::ServerHandler(QObject *parent)
    : QObject(parent)
    , m_crypto()
    , m_buffer(THORQ_PAYLOAD_LEN_MAX)
    , m_ApiConnection(nullptr)
{
    establishConnection();
}

ThorQ::ServerHandler::~ServerHandler()
{
}

ThorQ::Networking::Tcp::ApiConnection *ThorQ::ServerHandler::ApiConnection() const
{
    return m_ApiConnection;
}

void ThorQ::ServerHandler::resetState()
{
    m_crypto.reset();
    m_buffer.clear();
}

void ThorQ::ServerHandler::handleConnect(std::uint32_t data)
{
    qDebug() << "Connected";
}

void ThorQ::ServerHandler::handleDisconnect(std::uint32_t data)
{
    qDebug() << "Disconnected";

    establishConnection();
}

void ThorQ::ServerHandler::parsePacket(ThorQ::Networking::Buffer message)
{
    qDebug() << "Packet";
    ENetPacket* packet = message.packet();
    std::uint8_t channelID = message.channelID();

    if (packet == nullptr ||
        channelID > (std::uint8_t)THORQ_CHANNEL::_MAX  ||
        !ThorQ::checkDataSize(packet))
    {
        return;
    }

    m_buffer.resize(ThorQ::calculateDataSize(packet));
    if (!ThorQ::dataDecode(packet, m_buffer, m_crypto))
    {
        qDebug() << "Cannot unpack/decrypt packet";
        return;
    }

    auto fbsMessage = flatbuffers::GetRoot<ThorQ::Serialization::Message>(m_buffer.data());
    auto fbsVerifier = flatbuffers::Verifier(m_buffer.data(), m_buffer.size());

    if (!fbsMessage->Verify(fbsVerifier))
    {
        qDebug() << "Invalid flatbuffer message";
        return;
    }

    switch ((THORQ_CHANNEL)channelID) {
    case THORQ_CHANNEL::API:
        break;
    case THORQ_CHANNEL::EVENTS:
        break;
    case THORQ_CHANNEL::RTC:
        break;
    case THORQ_CHANNEL::AUTHORITY:
        break;
    default:
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        qDebug() << "[MSG] account";
        //handleMessageAccount(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_crypto:
        handleMessageCrypto(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_system_id:
        qDebug() << "[MSG] systemid";
        //handleMessageSystemID(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_group:
        qDebug() << "[MSG] group";
        //handleMessageGroup(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_device:
        qDebug() << "[MSG] device";
        //handleMessageCollar(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_moderation:
        qDebug() << "[MSG] moderation";
        //handleMessageModeration(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_friend_request:
        qDebug() << "[MSG] friend request";
        //handleMessageFriendRequest(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_file:
        qDebug() << "[MSG] file";
        //handleMessageFile(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_user:
        qDebug() << "[MSG] user";
        //handleMessageUser(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_announcement:
        qDebug() << "[MSG] announcement";
        //handleMessageAnnouncement(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_NONE:
        qDebug() << "[MSG] none";
    default:
        return;
    }
}

void ThorQ::ServerHandler::requestCrypto()
{
    qDebug() << "Request Crypto";

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsRequest = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Request).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsRequest);
    fbsBuilder.Finish(fbsMessage);

    sendPacket(fbsBuilder.GetBufferSpan(), false, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::API);
}

void ThorQ::ServerHandler::establishConnection()
{
    if (m_ApiConnection != nullptr) {
        m_ApiConnection->deleteLater();
    }

    m_ApiConnection = new Networking::Tcp::ApiConnection(this);
    QObject::connect(m_ApiConnection, &Networking::Tcp::ApiConnection::connected, this, &ServerHandler::handleConnect);
    QObject::connect(m_ApiConnection, &Networking::Tcp::ApiConnection::disconnected, this, &ServerHandler::handleDisconnect);
    QObject::connect(m_ApiConnection, &Networking::Tcp::ApiConnection::udpReceived, this, &ServerHandler::parsePacket);
    QObject::connect(this, &ServerHandler::packetGenerated, m_ApiConnection, &Networking::Tcp::ApiConnection::sendUdp);
    emit requestConnect(THORQ_SERVER_HOSTNAME, THORQ_SERVER_PORT, (std::uint8_t)THORQ_CHANNEL::_MAX, m_ApiConnection);
}

void ThorQ::ServerHandler::sendPacket(std::span<std::uint8_t> span, bool encrypt, std::uint32_t flags, THORQ_CHANNEL channelID)
{
    std::size_t packetSize = ThorQ::calculatePacketSize(span.size(), encrypt);
    ENetPacket* packet = enet_packet_create(nullptr, packetSize, flags);

    if (packet != nullptr && ThorQ::dataEncode(packet, span))
    {
        emit packetGenerated(ThorQ::Networking::Buffer(packet, (std::uint8_t)channelID));
    }
}

void ThorQ::ServerHandler::handleMessageVersion(const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsVersion = reinterpret_cast<const ThorQ::Serialization::Version*>(body);

    if (!fbsVersion->Verify(fbsVerifier)) {
        return;
    }

    qDebug() << "[MSG] version";

    ThorQ::Version version;
    version.setMajor(fbsVersion->major());
    version.setMinor(fbsVersion->minor());
    version.setPatch(fbsVersion->patch());

    switch ((THORQ_APP)fbsVersion->app()) {
    case THORQ_APP::SERVER:
        if (version == THORQ_VERSION_SERVER) {
            qDebug() << "[VERSION] Server version matched";
        } else {
            qDebug() << "[VERSION] Server version mismatched";
        }

        requestCrypto();
        break;
    case THORQ_APP::CLIENT:
        if (version == THORQ_VERSION_CLIENT) {
            qDebug() << "[VERSION] Client version matched";
        } else {
            qDebug() << "[VERSION] Client version mismatched";
        }
        break;
    case THORQ_APP::LINK:
        if (version == THORQ_VERSION_LINK) {
            qDebug() << "[VERSION] Link version matched";
        } else {
            qDebug() << "[VERSION] Link version mismatched";
        }
        break;
    default:
        qDebug() << "[VERSION] invalid";
        return;
    }
}

void ThorQ::ServerHandler::handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(body);

    if (!fbsCrypto->Verify(fbsVerifier)) {
        return;
    }

    qDebug() << "[MSG] crypto";

    switch (fbsCrypto->type()) {
    case ThorQ::Serialization::Crypto::MessageType_Establish:
    {
        qDebug() << "[MSG] crypto establish!";

        if (fbsCrypto->data()->size() != ThorQ::Crypto::PublicKeyLen)
        {
            qDebug() << "Got key with invalid length!";
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

            sendPacket(fbsBuilder.GetBufferSpan(), false, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::API);
        }
        else
        {
            qWarning() << "Failed to create shared secret";
            m_ApiConnection->disconnect((std::uint32_t)THORQ_DISCONNECT_REASON::CRYPTO_FAILED);
        }
        break;
    }
    case ThorQ::Serialization::Crypto::MessageType_Verify:
    {
        qDebug() << "[MSG] crypto verify!" << fbsCrypto->data()->size();

        if (fbsCrypto->data()->size() == THORQ_CRYPTO_VERIFICATION_DATA_LEN)
        {
            qDebug() << "[MSG] crypto verified!";

            // Build flatbuffer
            flatbuffers::FlatBufferBuilder fbsBuilder;
            auto fbsVerify  = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Verify, fbsBuilder.CreateVector(fbsCrypto->data()->data(), fbsCrypto->data()->size())).Union();
            auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsVerify);
            fbsBuilder.Finish(fbsMessage);

            sendPacket(fbsBuilder.GetBufferSpan(), true, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::API);
        }
        else
        {
            qWarning() << "Failed to verify";
            m_ApiConnection->disconnect((std::uint32_t)THORQ_DISCONNECT_REASON::CRYPTO_FAILED);
        }
        break;
    }
    default:
        qDebug() << "[MSG] crypto \?\?\?!\n";
        return;
    }
}
*/
