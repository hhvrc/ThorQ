#ifndef APICLIENT_H
#define APICLIENT_H

#include "typedefs_client.h"


#include <networking/tcpconnection.h>
#include <cryptography/encryption.h>
#include <messagehandlingcontext.h>
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

    void establishCrypto(HandlerContext& context);
    void onCryptoEstablished(HandlerContext& context);

    void handleMessage(HandlerContext& context);
    bool sendContextData(HandlerContext& context);

    void handleMessageAnnouncement(HandlerContext& context);
    void handleMessageDevice(HandlerContext& context);
    void handleMessageCrypto(HandlerContext& context);
    void handleMessageFile(HandlerContext& context);
    void handleMessageFriendRequest(HandlerContext& context);
    void handleMessageGroup(HandlerContext& context);
    void handleMessageModeration(HandlerContext& context);
    void handleMessageSystemID(HandlerContext& context);
    void handleMessageUser(HandlerContext& context);
    void handleMessageVersion(HandlerContext& context);
    void handleMessageP2P(HandlerContext& context);

    bool encodeAndSend(const std::span<std::uint8_t>& buffer, bool encrypt);

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
