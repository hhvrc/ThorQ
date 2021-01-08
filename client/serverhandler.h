#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H

#include <cstdint>
#include <vector>

#include <QUuid>
#include <QObject>
#include <QByteArray>
#include <QSharedPointer>

#include <typedefs_global.h>
#include "typedefs_client.h"

namespace ThorQ {
class ServerHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerHandler)
public:
    ServerHandler(ThorQ::Client* client, QObject* parent);
    ~ServerHandler();
signals:
    void requestConnection(QUuid connectionID, QString hostname, std::uint16_t port, std::uint8_t channelCount);
    void messageSend(QByteArray data, std::uint8_t channelID);
public slots:
    void reset();
private slots:
    void establishConnection();

    void connectionEstablished(ThorQ::ClientConnection* connection);
    void connectionFailed(QUuid connecitonID);

    void disconnected(std::uint8_t reason);

    void packetReceived(ThorQ::ClientMessage msg);
private:
    std::shared_ptr<ThorQ::Crypto> m_crypto;
    std::vector<std::uint8_t> m_buffer;

    ThorQ::Client* m_client;
    QUuid m_connectionID;
    ThorQ::ClientConnection* m_connection;
};
}

#endif // SERVERHANDLER_H
