#ifndef APICLIENT_H
#define APICLIENT_H

#include "typedefs_client.h"

#include <networking/tcpconnection.h>
#include <cryptography/encryption.h>
#include <cryptography/signer.h>
#include <concurrentqueue.h>
#include <enums.h>

#include <QObject>
#include <QTimer>

#include <thread>

namespace ThorQ {
class ApiClient : public QObject
{
    Q_OBJECT
public:
    ApiClient(QObject* parent = nullptr);
    ~ApiClient();

    ProcessStatus processStatus() const { return m_processStatus; }
    ConnectionStatus connectionStatus() const { return m_connectionStatus; }
    CryptoLinkStatus cryptoLinkStatus() const { return m_cryptoLinkStatus; }

    bool netConnect(QString host, quint16 port);
    void netDisconnect();
signals:
    void netConnected();
    void netDisconnected();

    void processStatusChanged(ProcessStatus status);
    void connectionStatusChanged(ConnectionStatus status);
    void cryptoLinkStatusChanged(CryptoLinkStatus status);

    void errorOccured(QString error);
private slots:
    void pollEvents();
private:
    bool setProcessStatus(ProcessStatus status);
    bool setConnectionStatus(ConnectionStatus status);
    bool setCryptoLinkStatus(CryptoLinkStatus status);

    void onError(const std::error_code& ec);
    void onConnect();
    void onDisconnect();
    void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message);

    void establishCrypto();
    void onCryptoEstablished();

    void handleMessageAccount(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageAnnouncement(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageDevice(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageFile(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageFriendRequest(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageGroup(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageModeration(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageSystemID(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageUser(const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageVersion(const void *body, flatbuffers::Verifier fbsVerifier);
    void handleMessageP2P(const void* body, flatbuffers::Verifier fbsVerifier);

    bool encodeAndSend(const flatbuffers::span<std::uint8_t>& buffer, bool encrypt);

    QTimer* m_pollTimer;

    asio::io_context m_asio;
    std::vector<std::thread> m_threads;

    ProcessStatus m_processStatus;
    ConnectionStatus m_connectionStatus;
    CryptoLinkStatus m_cryptoLinkStatus;

    std::shared_ptr<ThorQ::ApiClientConnection> m_connection;

    moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>> m_incomingMessages;
    moodycamel::ConsumerToken m_incomingMessagesToken;
    moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>> m_outgoingMessages;
    moodycamel::ProducerToken m_outgoingMessagesToken;

    ThorQ::Crypto::Signer m_signer;
    ThorQ::Crypto::Encryption m_crypto;
    std::vector<std::uint8_t> m_buffer;
};
}

#endif // APICLIENT_H
