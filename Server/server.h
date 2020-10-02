#ifndef SERVER_H
#define SERVER_H

#include <QThread>
#include <QSet>

#include "typedefs_global.h"

namespace ThorQ {
class Server : public QThread
{
    Q_OBJECT
public:
    static QString version();
    static bool Init();
    static void DeInit();

    Server(QObject* parent = nullptr);

    bool setup(std::uint16_t port, std::size_t maxPeers, std::uint8_t channelCount);
    void cleanup();

    void setNoDelay(bool enabled);

    std::uint16_t heartbeatInterval() const;

    std::size_t maxPacketSize() const;

    std::uint64_t totalDataSent() const;
    std::uint64_t totalPacketsSent() const;
    std::uint64_t totalDataReceived() const;
    std::uint64_t totalPacketsReceived() const;
signals:
    void enetEvent(ENetEvent event);
    void maxPacketSizeChanged(std::size_t size);
public slots:
    void setMaxPacketSize(std::size_t size);
    void setHearbeatInterval(std::uint16_t interval);
private slots:
    void handleEventNewConnection(const ENetEvent& event);
    void handleEventDisconnect(const ENetEvent& peer);
    void handleEventTimeout(const ENetEvent& peer);
private:
    void run() override;

    ENetHost* m_host;
    QSet<ENetPeer*> m_peers;

    std::uint16_t m_heartbeatInterval;

    std::uint64_t m_totalSentData;
    std::uint64_t m_totalSentPackets;

    std::uint64_t m_totalReceivedData;
    std::uint64_t m_totalReceivedPackets;
};
}

#endif // SERVER_H
