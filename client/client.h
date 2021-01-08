#ifndef CLIENT_H
#define CLIENT_H

#include <span>
#include <atomic>
#include <vector>
#include <shared_mutex>

#include <QObject>
#include <QUuid>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>

#include <QWeakPointer>
#include <QSharedPointer>

#include <enums.h>
#include <typedefs_global.h>

namespace ThorQ {
class ClientConnection;
class Client : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Client)

    Client(ENetHost* host, QObject* parent);
public:
    static const char* Version();
    static bool Initialize();
    static void DeInitialize();

    static ThorQ::Client* NewClient(std::uint8_t channelLimit, QObject* parent);
    static ThorQ::Client* NewClient(QString hostname, std::uint16_t port, std::size_t peerLimit, std::uint8_t channelLimit, QObject* parent);
    ~Client();

    QList<ThorQ::ClientConnection*> connections();

    std::uint64_t txData() const;
    std::uint32_t txSpeed() const;
    std::uint64_t rxData() const;
    std::uint32_t rxSpeed() const;
public slots:
    void connect(QUuid connectionID, QString hostname, std::uint16_t port, std::uint8_t channelCount);
signals:
    void connectionIncoming(ThorQ::ClientConnection* connection);
    void connectionEstablished(ThorQ::ClientConnection* connection);
    void connectionFailed(QUuid connectionID);

    void txDataChanged(std::uint64_t txData);
    void txSpeedChanged(std::uint32_t txSpeed);
    void rxDataChanged(std::uint64_t rxData);
    void rxSpeedChanged(std::uint32_t rxSpeed);

    void warning(const QString& what);
    void error(const QString& what);
private slots:
    void service();
    void updateStats();

    void handleEventConnection(const ENetEvent& event);
    void handleEventMessage(const ENetEvent &event);
    void handleEventTimeout(const ENetEvent& event);
    void handleEventDisconnect(const ENetEvent& event);
private:
    QTimer m_serviceTimer;
    QTimer m_statisticsTimer;

    QList<ThorQ::ClientConnection*> m_connections;

    ENetHost* m_host;

    std::atomic_uint64_t m_totalDataTx;
    std::atomic_uint64_t m_totalDataRx;
    std::atomic_uint32_t m_speedTx;
    std::atomic_uint32_t m_speedRx;
};

class ClientMessage
{
protected:
    friend ThorQ::ClientConnection;
    ClientMessage(ENetPacket* packet, std::uint8_t channelID);
public:
    ClientMessage();
    ClientMessage(const ClientMessage& other);
    ClientMessage& operator=(const ClientMessage& other);
    ~ClientMessage();

    std::shared_ptr<ENetPacket> packet() const;
    std::uint8_t channelID() const;
private:
    std::shared_ptr<ENetPacket> m_packet;
    std::uint8_t m_channelID;
};

class ClientConnection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ClientConnection)
protected:
    friend ThorQ::Client;
    ClientConnection(QUuid id, ENetPeer* peer, ThorQ::Client* client);
protected slots:
    ENetPeer* peer() const;
    void clear();
    void updateStats();
    void handleEventMessage(const ENetEvent &event);
    void handleEventTimeout(const ENetEvent& event);
    void handleEventDisconnect(const ENetEvent& event);
public:
    ~ClientConnection();

    QUuid id() const;

    bool connected() const;

    std::uint16_t rtt() const;
    std::uint64_t txData() const;
    std::uint32_t txSpeed() const;
    std::uint64_t rxData() const;
    std::uint32_t rxSpeed() const;
signals:
    void rttChanged(std::uint16_t rtt);
    void txDataChanged(std::uint64_t txData);
    void txSpeedChanged(std::uint32_t txSpeed);
    void rxDataChanged(std::uint64_t rxData);
    void rxSpeedChanged(std::uint32_t rxSpeed);

    void packetReceived(ClientMessage message);

    void disconnected(std::uint32_t reason);
public slots:
    void packetSend(QByteArray data, std::uint8_t channel);
    void packetSendReliable(QByteArray data, std::uint8_t channel);
    void packetSendSequenced(QByteArray data, std::uint8_t channel);

    void disconnect(std::uint32_t reason);
    void disconnectNow(std::uint32_t reason);
    void disconnectLater(std::uint32_t reason);
private:
    void sendPacket(const QByteArray& data, std::uint32_t flags, std::uint8_t channel);

    QUuid m_id;

    ENetPeer* m_peer;

    std::atomic_uint64_t m_totalDataTx;
    std::atomic_uint64_t m_totalDataRx;
    std::atomic_uint32_t m_speedTx;
    std::atomic_uint32_t m_speedRx;
    std::atomic_uint16_t m_rtt;
};
}
Q_DECLARE_METATYPE(ThorQ::ClientMessage)

#endif // CLIENT_H
