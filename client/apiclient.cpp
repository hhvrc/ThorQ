#include "apiclient.h"

#include "apiclient_connection.h"

#include <systemid.h>
#include <encoding.h>
#include <enums.h>

#include <schemas_common.h>

#include <fmt/core.h>

#include <mutex>

ThorQ::ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
    , m_asio((int)std::thread::hardware_concurrency())
    , m_threads()
    , m_processStatus(ProcessStatus::Stopped)
    , m_connectionStatus(ConnectionStatus::Disconnected)
    , m_connection(nullptr)
    , m_incomingMessages()
    , m_incomingMessagesToken(m_incomingMessages)
    , m_outgoingMessages()
    , m_outgoingMessagesToken(m_outgoingMessages)
    , m_signer()
    , m_crypto()
    , m_buffer(THORQ_PAYLOAD_LEN_MAX)
{
    QObject::connect(m_pollTimer, &QTimer::timeout, this, &ApiClient::pollEvents);
    m_pollTimer->setInterval(0);
    m_pollTimer->setSingleShot(false);

    fmt::print("[CLIENT] Constructed\n");
}

ThorQ::ApiClient::~ApiClient()
{
    netDisconnect();
    fmt::print("[CLIENT] Destroyed\n");
}

bool ThorQ::ApiClient::netConnect(QString host, quint16 port)
{
    ProcessStatus expected = processStatus();
    if (expected == ProcessStatus::Stopped)
    {
        setProcessStatus(ProcessStatus::Starting);
        std::string hostStdString = host.toStdString();

        try
        {
            m_connection = std::make_shared<ThorQ::ApiClientConnection>(m_asio, asio::ip::tcp::socket(m_asio), m_incomingMessages, m_outgoingMessages);

            asio::ip::tcp::resolver resolver(m_asio);
            auto endpoints = resolver.resolve(hostStdString, std::to_string(port));

            m_connection->connect(endpoints);
        }
        catch (std::exception& ex)
        {
            m_connection = nullptr;
            fmt::print(stderr, "[CLIENT] Failed to connect: {}\n", ex.what());
            return false;
        }

        fmt::print("[CLIENT] Connecting to {}[{}]...\n", hostStdString, port);
        std::uint32_t nproc = std::thread::hardware_concurrency();
        for (std::uint32_t i = 0; i < nproc; i++)
        {
            m_threads.emplace_back([this](){ m_asio.run(); });
        }

        m_pollTimer->start();

        setProcessStatus(ProcessStatus::Running);
        return true;
    }

    return expected == ProcessStatus::Starting || expected == ProcessStatus::Running;
}

void ThorQ::ApiClient::netDisconnect()
{
    if (processStatus() == ProcessStatus::Running)
    {
        setProcessStatus(ProcessStatus::Stopping);
        m_pollTimer->stop();

        m_asio.stop();

        for (auto it = m_threads.begin(); it != m_threads.end(); it++)
        {
            if (it->joinable())
            {
                it->join();
            }
        }

        m_threads.clear();
        m_connection = nullptr;

        setProcessStatus(ProcessStatus::Stopped);
    }
}

void ThorQ::ApiClient::pollEvents()
{
    auto connection = m_connection;

    if (connection == nullptr) {
        return;
    }

    std::shared_ptr<std::vector<std::uint8_t>> message;
    while (m_incomingMessages.try_dequeue(m_incomingMessagesToken, message)) {
        onMessage(message);
    }

    ConnectionStatus status = connection->status();
    if (setConnectionStatus(status)) {
        switch (status) {
        case ConnectionStatus::Error:
            emit errorOccured(
                        QString::fromStdString(
                            m_connection->latestErrorCode().message()));
            break;
        case ConnectionStatus::Disconnected:
            onDisconnect();
            emit netDisconnected();
            m_connection = nullptr;
            return;
        case ConnectionStatus::Connected:
            onConnect();
            emit netConnected();
            break;
        case ConnectionStatus::Connecting:
        case ConnectionStatus::Disconnecting:
        default: // UHMMMM
            break;
        }
    }
}

bool ThorQ::ApiClient::setProcessStatus(ProcessStatus status)
{
    if (m_processStatus != status) {
        m_processStatus = status;
        emit processStatusChanged(status);
        return true;
    }
    return false;
}

bool ThorQ::ApiClient::setConnectionStatus(ConnectionStatus status)
{
    if (m_connectionStatus != status) {
        m_connectionStatus = status;
        emit connectionStatusChanged(status);
        return true;
    }
    return false;
}

void ThorQ::ApiClient::onConnect()
{
    establishCrypto();
}

void ThorQ::ApiClient::onDisconnect()
{

}

void ThorQ::ApiClient::onMessage(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    fmt::print("[CLIENT] Message\n");

    m_buffer.resize(ThorQ::Encoding::calculateDataSize(message->data(), message->size()));
    if (!ThorQ::Encoding::messageDecode(message->data(), message->size(), m_buffer.data(), m_buffer.size(), m_crypto))
    {
        fmt::print(stderr, "Cannot unpack/decrypt packet\n");
        disconnect();
        return;
    }

    auto fbsMessage = flatbuffers::GetRoot<ThorQ::Serialization::Message>(m_buffer.data());
    auto fbsVerifier = flatbuffers::Verifier(m_buffer.data(), m_buffer.size());

    if (!fbsMessage->Verify(fbsVerifier))
    {
        fmt::print(stderr, "Invalid flatbuffer message\n");
        disconnect();
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        handleMessageAccount(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_announcement:
        handleMessageAnnouncement(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_device:
        handleMessageDevice(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_crypto:
        handleMessageCrypto(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_file:
        handleMessageFile(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_friend_request:
        handleMessageFriendRequest(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_group:
        handleMessageGroup(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_moderation:
        handleMessageModeration(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_system_id:
        handleMessageSystemID(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_user:
        handleMessageUser(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_p2p:
        handleMessageP2P(fbsMessage->body(), fbsVerifier);
        break;
    default:
        fmt::print("[MSG] Invalid\n");
        return;
    }
}

void ThorQ::ApiClient::onCryptoEstablished()
{
    fmt::print("[CONNECTION] Crypto established!\n");
}

void ThorQ::ApiClient::establishCrypto()
{
    fmt::print("[MSG] Crypto\n");

    if (!m_crypto.generateKeyPair()) {
        fmt::print(stderr, "[CRYPTO] Failed to generate keypair!\n");
        return;
    }

    auto myPk = m_crypto.publicKey();

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsPublicKey = fbsBuilder.CreateVector(myPk.data(), myPk.size());
    auto fbsCryptoCli = ThorQ::Serialization::Crypto::CreateEstablishCryptoClient(fbsBuilder, fbsPublicKey).Union();
    auto fbsRequest   = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::Body_establish_crypto_client, fbsCryptoCli).Union();
    auto fbsMessage   = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsRequest);
    fbsBuilder.Finish(fbsMessage);

    encodeAndSend(fbsBuilder.GetBufferSpan(), false);
}

void ThorQ::ApiClient::handleMessageAccount(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Account\n");
}

void ThorQ::ApiClient::handleMessageAnnouncement(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Announcement\n");
}

void ThorQ::ApiClient::handleMessageDevice(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Device\n");
}

void ThorQ::ApiClient::handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Crypto\n");

    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(body);

    if (!fbsCrypto->Verify(fbsVerifier)) {
        return;
    }

    switch (fbsCrypto->body_type()) {
    case ThorQ::Serialization::Crypto::Body_establish_crypto_server:
    {
        fmt::print("[CRYPTO] Establish!\n");

        auto fbsEstablishServer = fbsCrypto->body_as_establish_crypto_server();
        auto fbsServerPublicKey = fbsEstablishServer->public_key();
        auto fbsServerSignature = fbsEstablishServer->signature();

        if (fbsServerPublicKey->size() != ThorQ::Crypto::Encryption::PublicKeyLen ||
            fbsServerSignature->size() != ThorQ::Crypto::Signer::SignatureLen)
        {
            fmt::print(stderr, "[CRYPTO] Got key of invalid size!\n");
            return;
        }

        ThorQ::Crypto::Signer serverVerifier;
        serverVerifier.setPublicKey(ThorQ::Crypto::Signer::RootSigner()); // TODO: Load from file

        if (!serverVerifier.verify(fbsServerPublicKey->data(), fbsServerPublicKey->size(), fbsServerSignature->data(), fbsServerSignature->size()))
        {
            fmt::print(stderr, "[CRYPTO] Failed to verify server key validity!\n");
            disconnect();
            return;
        }

        // Set foreign key
        if (!m_crypto.setForeignKey(fbsServerPublicKey->data(), fbsServerPublicKey->size()))
        {
            fmt::print(stderr, "[CRYPTO] Failed to generate keypair!\n");
            return;
        }

        onCryptoEstablished();
        break;
    }
    case ThorQ::Serialization::Crypto::Body_update_server_sign_key:
    {
        fmt::print("[CRYPTO] Update server signing key\n");

        break;
    }
    case ThorQ::Serialization::Crypto::Body_update_server_encrypt_key:
    {
        fmt::print("[CRYPTO] Update server encryption key\n");

        auto fbsUpdateEncryptPk = fbsCrypto->body_as_establish_crypto_server();
        auto fbsServerPublicKey = fbsUpdateEncryptPk->public_key();
        auto fbsServerSignature = fbsUpdateEncryptPk->signature();

        if (fbsServerPublicKey->size() != ThorQ::Crypto::Encryption::PublicKeyLen ||
            fbsServerSignature->size() != ThorQ::Crypto::Signer::SignatureLen)
        {
            fmt::print(stderr, "[CRYPTO] Got key of invalid size!\n");
            return;
        }

        ThorQ::Crypto::Signer serverVerifier;
        serverVerifier.setPublicKey(ThorQ::Crypto::Signer::RootSigner());

        if (!serverVerifier.verify(fbsServerPublicKey->data(), fbsServerPublicKey->size(), fbsServerSignature->data(), fbsServerSignature->size()))
        {
            fmt::print(stderr, "[CRYPTO] Failed to verify server key validity!\n");
            disconnect();
            return;
        }

        fmt::print("[CRYPTO] Updating server decryption key\n");

        // Build flatbuffer
        /*
        flatbuffers::FlatBufferBuilder fbsBuilder;
        auto fbsVerify  = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Verify, fbsBuilder.CreateVector(fbsCrypto->data()->data(), fbsCrypto->data()->size())).Union();
        auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsVerify);
        fbsBuilder.Finish(fbsMessage);

        encodeAndSend(fbsBuilder.GetBufferSpan(), true);
        */
        break;
    }
    default:
        fmt::print("[CRYPTO] \?\?\?!\n\n");
        return;
    }
}

void ThorQ::ApiClient::handleMessageFile(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] File\n");
}

void ThorQ::ApiClient::handleMessageFriendRequest(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Friend request\n");
}

void ThorQ::ApiClient::handleMessageGroup(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Group\n");
}

void ThorQ::ApiClient::handleMessageModeration(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Moderation\n");
}

void ThorQ::ApiClient::handleMessageSystemID(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Systemid\n");
}

void ThorQ::ApiClient::handleMessageUser(const void *body, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] User\n");
}

void ThorQ::ApiClient::handleMessageVersion(const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsVersion = reinterpret_cast<const ThorQ::Serialization::Version*>(body);

    if (!fbsVersion->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] version\n");

    ThorQ::Version version;
    version.setMajor(fbsVersion->major());
    version.setMinor(fbsVersion->minor());
    version.setPatch(fbsVersion->patch());

    switch ((THORQ_APP)fbsVersion->app()) {
    case THORQ_APP::SERVER:
        if (version == ThorQ::ServerVersion) {
            fmt::print("[VERSION] Server version matched\n");
        } else {
            fmt::print("[VERSION] Server version mismatched\n");
        }
        break;
    case THORQ_APP::CLIENT:
        if (version == ThorQ::ClientVersion) {
            fmt::print("[VERSION] Client version matched\n");
        } else {
            fmt::print("[VERSION] Client version mismatched\n");
        }
        break;
    case THORQ_APP::LINK:
        if (version == ThorQ::LinkVersion) {
            fmt::print("[VERSION] Link version matched\n");
        } else {
            fmt::print("[VERSION] Link version mismatched\n");
        }
        break;
    default:
        fmt::print("[VERSION] invalid\n");
        return;
    }
}

void ThorQ::ApiClient::handleMessageP2P(const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsP2P = reinterpret_cast<const ThorQ::Serialization::Peer2Peer::Message*>(body);

    if (!fbsP2P->Verify(fbsVerifier)) {
        return;
    }
}

void ThorQ::ApiClient::encodeAndSend(const flatbuffers::span<std::uint8_t>& buffer, bool encrypt)
{
    auto message = std::make_shared<std::vector<std::uint8_t>>();
    message->resize(ThorQ::Encoding::calculateMessageSize(buffer.size(), encrypt));

    if (encrypt) {
        if (!ThorQ::Encoding::messageEncode(buffer.data(), buffer.size(), message->data(), message->size(), m_crypto)) {
            fmt::print(stderr, "Failed to encrypt message\n");
            return;
        }
    }
    else {
        if (!ThorQ::Encoding::messageEncode(buffer.data(), buffer.size(), message->data(), message->size())) {
            fmt::print(stderr, "Failed to encode message\n");
            return;
        }
    }

    if (m_connection != nullptr) {
        m_connection->messageSend(message);
    }
}
