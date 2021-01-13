#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <cstdint>
#include <vector>
#include <array>

#include <QUuid>
#include <QObject>
#include <QByteArray>
#include <QSharedPointer>

#include <enums.h>
#include <crypto.h>
#include <constants.h>
#include <typedefs_global.h>
#include "typedefs_client.h"

namespace ThorQ {
class ServerHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerHandler)
public:
    ServerHandler(QObject* parent);
    ~ServerHandler();
signals:
    void requestConnect();
    void requestDisconnect(std::uint32_t reason);

    void packetGenerated(ThorQ::Networking::Message message);
public slots:
    void resetState();
    void parsePacket(ThorQ::Networking::Message message);
private:
    void sendPacket(std::span<std::uint8_t> span, bool encrypt, std::uint32_t flags, THORQ_CHANNEL channelID);
    void handleMessageVersion(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageUser(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageFile(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageSystemID(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageAccount(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageFriendRequest(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageGroup(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageModeration(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageCollar(const void* body, flatbuffers::Verifier fbsVerifier);

    QUuid m_connectionID;
    ThorQ::Crypto m_crypto;
    std::vector<std::uint8_t> m_buffer;
};
}

#endif // SERVERHANDLER_H
