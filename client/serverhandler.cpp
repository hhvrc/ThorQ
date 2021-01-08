#include "serverhandler.h"

#include <QDebug>

#include <constants.h>
#include <crypto.h>
#include <thorq_message.h>

#include <schemas/message_generated.h>

#include "client.h"

ThorQ::ServerHandler::ServerHandler(ThorQ::Client* client, QObject *parent)
    : QObject(parent)
    , m_crypto(new ThorQ::Crypto())
    , m_buffer(THORQ_PAYLOAD_LEN_MAX)
    , m_client(client)
    , m_connectionID()
    , m_connection()
{
    QObject::connect(this, &ThorQ::ServerHandler::requestConnection, m_client, &ThorQ::Client::connect, Qt::QueuedConnection);
    QObject::connect(m_client, &ThorQ::Client::connectionEstablished, this, &ThorQ::ServerHandler::connectionEstablished, Qt::QueuedConnection);
    QObject::connect(m_client, &ThorQ::Client::connectionFailed, this, &ThorQ::ServerHandler::connectionFailed, Qt::QueuedConnection);
    reset();
    establishConnection();
}

ThorQ::ServerHandler::~ServerHandler()
{
}

void ThorQ::ServerHandler::reset()
{
    m_crypto->reset();
    m_connectionID = QUuid::createUuid();
}

void ThorQ::ServerHandler::establishConnection()
{
    reset();
    emit requestConnection(m_connectionID, "localhost", 12345, 8);
}

void ThorQ::ServerHandler::connectionEstablished(ThorQ::ClientConnection* connection)
{
    QObject::connect(connection, &ThorQ::ClientConnection::disconnected, this, &ThorQ::ServerHandler::disconnected, Qt::QueuedConnection);
    QObject::connect(connection, &ThorQ::ClientConnection::destroyed,    this, &ThorQ::ServerHandler::establishConnection, Qt::QueuedConnection);
    QObject::connect(connection, &ThorQ::ClientConnection::packetReceived, this, &ThorQ::ServerHandler::packetReceived, Qt::QueuedConnection);

}

void ThorQ::ServerHandler::connectionFailed(QUuid connecitonID)
{
    if (connecitonID == m_connectionID)
    {
        establishConnection();
    }
}

void ThorQ::ServerHandler::disconnected(std::uint8_t reason)
{
    qDebug() << "Disconnected" << reason;
    establishConnection();
}

void ThorQ::ServerHandler::packetReceived(ThorQ::ClientMessage msg)
{
    std::shared_ptr<ENetPacket> packet = msg.packet();
    std::uint8_t channelID = msg.channelID();

    if (packet == nullptr ||
        channelID > (std::uint8_t)THORQ_CHANNEL::_MAX  ||
        !ThorQ::packetIsValidSize(packet.get()))
    {
        return;
    }

    m_buffer.resize(ThorQ::calculateDataSize(packet.get()));
    if (!ThorQ::packetDecode(packet.get(), m_buffer, m_crypto))
    {
        return;
    }

    auto fbsMessage = flatbuffers::GetRoot<ThorQ::Serialization::Message>(m_buffer.data());
    auto fbsVerifier = flatbuffers::Verifier(m_buffer.data(), m_buffer.size());

    if (!fbsMessage->Verify(fbsVerifier))
    {
        return;
    }

    switch ((THORQ_CHANNEL)channelID) {
    case THORQ_CHANNEL::MAIN:
        break;
    case THORQ_CHANNEL::EVENTS:
        break;
    case THORQ_CHANNEL::STREAM:
        break;
    case THORQ_CHANNEL::AUTHORITY:
        break;
    case THORQ_CHANNEL::_MAX:
    case THORQ_CHANNEL::_INVALID:
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        qDebug() << "[MSG] account";
        //handleMessageAccount(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_version:
        qDebug() << "[MSG] version";
        //handleMessageVersion(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_crypto:
        qDebug() << "[MSG] crypto";
        //handleMessageCrypto(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_system_id:
        qDebug() << "[MSG] systemid";
        //handleMessageSystemID(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_group:
        qDebug() << "[MSG] group";
        //handleMessageGroup(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_collar:
        qDebug() << "[MSG] collar";
        //handleMessageCollar(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_moderation:
        qDebug() << "[MSG] moderation";
        //handleMessageModeration(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_friend_request:
        qDebug() << "[MSG] friend request";
        //handleMessageFriendRequest(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_file:
        qDebug() << "[MSG] file";
        //handleMessageFile(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_user:
        qDebug() << "[MSG] user";
        //handleMessageUser(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_announcement:
        qDebug() << "[MSG] announcement";
        //handleMessageAnnouncement(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_NONE:
        qDebug() << "[MSG] none";
    default:
        return;
    }
}
