#include "serverhandler.h"

#include <QDebug>

#include <enet.h>
#include <constants.h>
#include <thorq_message.h>

#include <schemas/message_generated.h>
#include <schemas/friendrequest_generated.h>

#include "networking/connectionhandler.h"

ThorQ::ServerHandler::ServerHandler(QObject *parent)
    : QObject(parent)
    , m_crypto()
    , m_buffer(THORQ_PAYLOAD_LEN_MAX)
    , m_connectionHandler(nullptr)
{
    establishConnection();
}

ThorQ::ServerHandler::~ServerHandler()
{
}

ThorQ::Networking::ConnectionHandler *ThorQ::ServerHandler::connectionHandler() const
{
    return m_connectionHandler;
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

void ThorQ::ServerHandler::parsePacket(ThorQ::Networking::Message message)
{
    qDebug() << "Packet";
    ENetPacket* packet = message.packet();
    std::uint8_t channelID = message.channelID();

    if (packet == nullptr ||
        channelID > (std::uint8_t)THORQ_CHANNEL::_MAX  ||
        !ThorQ::packetIsValidSize(packet))
    {
        return;
    }

    m_buffer.resize(ThorQ::calculateDataSize(packet));
    if (!ThorQ::packetDecode(packet, m_buffer, m_crypto))
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
        qDebug() << "[MSG] version";
        //handleMessageVersion(fbsMessage->body(), fbsVerifier);
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
    case ThorQ::Serialization::Body_collar:
        qDebug() << "[MSG] collar";
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

void ThorQ::ServerHandler::establishConnection()
{
    if (m_connectionHandler != nullptr) {
        m_connectionHandler->deleteLater();
    }

    m_connectionHandler = new Networking::ConnectionHandler(this);
    QObject::connect(m_connectionHandler, &Networking::ConnectionHandler::connected, this, &ServerHandler::handleConnect);
    QObject::connect(m_connectionHandler, &Networking::ConnectionHandler::disconnected, this, &ServerHandler::handleDisconnect);
    QObject::connect(m_connectionHandler, &Networking::ConnectionHandler::udpReceived, this, &ServerHandler::parsePacket);
    emit requestConnect(THORQ_SERVER_HOSTNAME, THORQ_SERVER_PORT, (std::uint8_t)THORQ_CHANNEL::_MAX, m_connectionHandler);
}

void ThorQ::ServerHandler::sendPacket(std::span<std::uint8_t> span, bool encrypt, std::uint32_t flags, THORQ_CHANNEL channelID)
{
    std::size_t packetSize = ThorQ::calculatePacketSize(span.size(), encrypt);
    ENetPacket* packet = enet_packet_create(nullptr, packetSize, flags);

    if (packet != nullptr && ThorQ::packetEncode(packet, span))
    {
        emit packetGenerated(ThorQ::Networking::Message(packet, (std::uint8_t)channelID));
    }
}

void ThorQ::ServerHandler::handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(body);

    if (!fbsCrypto->Verify(fbsVerifier))
    {
        return;
    }

    qDebug() << "[MSG] crypto";

    switch (fbsCrypto->type()) {
    case ThorQ::Serialization::Crypto::MessageType_Establish:
    {
        qDebug() << "[MSG] crypto establish!\n";

        if (fbsCrypto->data()->size() != ThorQ::Crypto::PublicKeyLen)
        {
            qDebug() << "Got key with invalid length!\n";
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
            emit requestDisconnect((std::uint32_t)THORQ_DISCONNECT_REASON::CRYPTO_FAILED);
        }
        break;
    }
    case ThorQ::Serialization::Crypto::MessageType_Verify:
    {
        qDebug() << "[MSG] crypto verify!";

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
            emit requestDisconnect((std::uint32_t)THORQ_DISCONNECT_REASON::CRYPTO_FAILED);
        }
        break;
    }
    default:
        qDebug() << "[MSG] crypto \?\?\?!\n";
        return;
    }
}
