#ifndef APICLIENT_H
#define APICLIENT_H

#include "typedefs_client.h"

#include <networking/tcpconnection.h>
#include <cryptography/encryption.h>
#include <cryptography/signer.h>
#include <concurrentqueue.h>
#include <enums.h>

#include <flatbuffers/flatbuffers.h>

#include <QObject>
#include <QTimer>
#include <QSettings>

#include <thread>

namespace ThorQ {
class AccountController;
class ApiClient : public QObject
{
    Q_OBJECT
public:
    ApiClient(QObject* parent = nullptr);
    ~ApiClient();

    ProcessStatus processStatus() const { return m_processStatus; }
    ConnectionStatus connectionStatus() const { return m_connectionStatus; }

    ThorQ::AccountController* accountController() const { return m_accountController; };
signals:
    void netConnected();
    void netDisconnected();

    void processStatusChanged(ProcessStatus status);
    void connectionStatusChanged(ConnectionStatus status);

    void errorOccured(QString error);
public slots:
    void netConnect();
    void netDisconnect();
private slots:
    void pollEvents();
private:
    bool setProcessStatus(ProcessStatus status);
    bool setConnectionStatus(ConnectionStatus status);

    void onError(const std::error_code& ec);
    void onConnect();
    void onDisconnect();
    void onMessage(std::shared_ptr<std::vector<std::uint8_t>> message);

    void establishCrypto();
    void onCryptoEstablished();

    void handleMessage(const void* body);
    void handleMessageAnnouncement(const void* body);
    void handleMessageDevice(const void* body);
    void handleMessageCrypto(const void* body);
    void handleMessageFile(const void* body);
    void handleMessageFriendRequest(const void* body);
    void handleMessageGroup(const void* body);
    void handleMessageModeration(const void* body);
    void handleMessageSystemID(const void* body);
    void handleMessageUser(const void* body);
    void handleMessageVersion(const void *body);
    void handleMessageP2P(const void* body);

    void encodeAndSend(const std::span<std::uint8_t>& buffer, bool encrypt);

    QTimer* m_pollTimer;
    QSettings m_settings;

    asio::io_context m_asio;
    std::vector<std::thread> m_threads;

    ProcessStatus m_processStatus;
    ConnectionStatus m_connectionStatus;

    std::shared_ptr<ThorQ::ApiClientConnection> m_connection;

    moodycamel::ConcurrentQueue<std::shared_ptr<std::vector<std::uint8_t>>> m_incomingMessages;
    moodycamel::ConsumerToken m_incomingMessagesToken;

    ThorQ::Crypto::Signer m_signer;
    ThorQ::Crypto::Encryption m_crypto;
    std::vector<std::uint8_t> m_buffer;

    ThorQ::AccountController* m_accountController;
};
}

#endif // APICLIENT_H
