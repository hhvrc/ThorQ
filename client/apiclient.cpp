#include "apiclient.h"

#include "accountcontroller.h"
#include "apiclient_connection.h"
#include "messagecontext.h"

#include <systemid.h>
#include <encoding.h>
#include <enums.h>

#include <schemas_common.h>

#include <fmt/core.h>

#include <mutex>

ThorQ::ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
    , m_settings(this)
    , m_asio((int)std::thread::hardware_concurrency())
    , m_threads()
    , m_processStatus(ProcessStatus::Stopped)
    , m_connectionStatus(ConnectionStatus::Disconnected)
    , m_connection(nullptr)
    , m_incomingMessages()
    , m_incomingMessagesToken(m_incomingMessages)
    , m_signer()
    , m_crypto()
    , m_buffer(THORQ_PAYLOAD_LEN_MAX)
    , m_requestCounter(0)
    , m_accountController(new ThorQ::AccountController(std::bind(&ThorQ::ApiClient::sendContextData, this, std::placeholders::_1), this))
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

void ThorQ::ApiClient::netConnect()
{
    ProcessStatus expected = processStatus();
    if (expected == ProcessStatus::Stopped)
    {
        setProcessStatus(ProcessStatus::Starting);
        auto hostname = m_settings.value("server/hostname").toString().toStdString();
        auto port = m_settings.value("server/port").toUInt();

        try
        {
            m_connection = std::make_shared<ThorQ::ApiClientConnection>(m_asio, asio::ip::tcp::socket(m_asio), m_incomingMessages);

            asio::ip::tcp::resolver resolver(m_asio);
            auto endpoints = resolver.resolve(hostname, std::to_string(port));

            m_connection->connect(endpoints);
        }
        catch (std::exception& ex)
        {
            m_connection = nullptr;
            emit errorOccured("Failed to connect: " + QString::fromStdString(ex.what()));
            emit netDisconnected();
            return;
        }

        std::uint32_t nproc = std::thread::hardware_concurrency();
        for (std::uint32_t i = 0; i < nproc; i++)
        {
            m_threads.emplace_back([this](){ m_asio.run(); });
        }

        m_pollTimer->start();

        setProcessStatus(ProcessStatus::Running);
    }
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
            onError(m_connection->latestErrorCode());
            break;
        case ConnectionStatus::Disconnected:
            onDisconnect();
            break;
        case ConnectionStatus::Connected:
            onConnect();
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

void ThorQ::ApiClient::onError(const std::error_code& ec)
{
    setConnectionStatus(ConnectionStatus::Error);
    emit errorOccured(QString::fromStdString(ec.message()));
}

void ThorQ::ApiClient::onConnect()
{
    MessageContext context;
    establishCrypto(context);
    sendContextData(context);

    emit netConnected();
}

void ThorQ::ApiClient::onDisconnect()
{
    emit netDisconnected();
    m_connection = nullptr;
}

void ThorQ::ApiClient::onMessage(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (!ThorQ::Encoding::isMessageValid(message->data(), message->size()))
    {
        fmt::print(stderr, "Received packet is corrupted\n");
        m_connection->disconnect();
        return;
    }

    m_buffer.resize(ThorQ::Encoding::calculateDataSize(message->data(), message->size()));
    if (!ThorQ::Encoding::messageDecode(message->data(), message->size(), m_buffer.data(), m_buffer.size(), m_crypto))
    {
        fmt::print(stderr, "Cannot unpack/decrypt packet\n");
        m_connection->disconnect();
        return;
    }

    auto fbsMessageBuffer = flatbuffers::GetRoot<ThorQ::Serialization::MessageBuffer>(m_buffer.data());
    auto fbsVerifier = flatbuffers::Verifier(m_buffer.data(), m_buffer.size());

    if (!fbsMessageBuffer->Verify(fbsVerifier))
    {
        fmt::print(stderr, "Invalid flatbuffer message\n");
        m_connection->disconnect();
        return;
    }

    if (fbsMessageBuffer->body() == nullptr) {
        fmt::print(stderr, "Body is nullptr\n");
        m_connection->disconnect();
        return;
    }

    auto& messages = *fbsMessageBuffer->body();

    MessageContext context;

    for (const auto& fbsMessage : messages) {
        context.body = fbsMessage;

        if (context.body != nullptr) {
            handleMessage(context);
        }
    }

    // Send all the queued data to the server
    if (!context.messages.empty()) {
        sendContextData(context);
    }
}

void ThorQ::ApiClient::establishCrypto(MessageContext& context)
{
    fmt::print("EstablishCrypto\n");

    if (!m_crypto.generateKeyPair()) {
        fmt::print(stderr, "[CRYPTO] Failed to generate keypair!\n");
        m_connection->disconnect();
        return;
    }

    auto fbsClientKey = context.fbsBuilder.CreateStruct(ThorQ::Serialization::Crypto::ClientKey(m_crypto.publicKey())).Union();
    auto fbsCrypto    = ThorQ::Serialization::Crypto::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Crypto::Body_client_key, fbsClientKey).Union();
    auto fbsMessage   = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_crypto, fbsCrypto, m_requestCounter++);

    context.messages.push_back(fbsMessage);

    context.encrypt = false;
}

void ThorQ::ApiClient::onCryptoEstablished(MessageContext& context)
{
    fmt::print("[CONNECTION] Crypto established!\n");

    std::vector<std::uint8_t> systemID = ThorQ::SystemID::systemid_generate();
    fmt::print("Sending SystemID: {}\n", ThorQ::SystemID::systemid_to_string(systemID));

    flatbuffers::Offset<ThorQ::Serialization::Version> fbsVersion;

    // Link version
    fbsVersion = ThorQ::Serialization::CreateVersion(context.fbsBuilder, (std::uint8_t)THORQ_APP::LINK, THORQ_VERSION_LINK_MAJOR, THORQ_VERSION_LINK_MINOR, THORQ_VERSION_LINK_PATCH);
    context.messages.push_back(ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union(), m_requestCounter++));

    // Client version
    fbsVersion = ThorQ::Serialization::CreateVersion(context.fbsBuilder, (std::uint8_t)THORQ_APP::CLIENT, THORQ_VERSION_CLIENT_MAJOR, THORQ_VERSION_CLIENT_MINOR, THORQ_VERSION_CLIENT_PATCH);
    context.messages.push_back(ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union(), m_requestCounter++));

    // Server version
    fbsVersion = ThorQ::Serialization::CreateVersion(context.fbsBuilder, (std::uint8_t)THORQ_APP::SERVER, THORQ_VERSION_SERVER_MAJOR, THORQ_VERSION_SERVER_MINOR, THORQ_VERSION_SERVER_PATCH);
    context.messages.push_back(ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union(), m_requestCounter++));

    // Hardware ID
    auto fbsVector   = context.fbsBuilder.CreateVector(systemID.data(), systemID.size());
    auto fbsSystemID = ThorQ::Serialization::SystemId::CreateMessage(context.fbsBuilder, ThorQ::Serialization::SystemId::Command_Submit, fbsVector).Union();
    auto fbsMessage  = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_system_id, fbsSystemID, m_requestCounter++);

    context.messages.push_back(fbsMessage);
}

void ThorQ::ApiClient::handleMessage(MessageContext& context)
{
    auto fbsMessage = static_cast<const ThorQ::Serialization::Message*>(context.body);

    context.body = fbsMessage->body();
    context.requestId = fbsMessage->request_id();

    if (context.body == nullptr) {
        fmt::print("[MSG] got null! ({})\n", context.requestId);
        return;
    }

    fmt::print("[MSG] {}\n", context.requestId);

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        m_accountController->ParseMessage(context);
        break;
    case ThorQ::Serialization::Body_announcement:
        handleMessageAnnouncement(context);
        break;
    case ThorQ::Serialization::Body_device:
        handleMessageDevice(context);
        break;
    case ThorQ::Serialization::Body_crypto:
        handleMessageCrypto(context);
        break;
    case ThorQ::Serialization::Body_file:
        handleMessageFile(context);
        break;
    case ThorQ::Serialization::Body_friend_request:
        handleMessageFriendRequest(context);
        break;
    case ThorQ::Serialization::Body_group:
        handleMessageGroup(context);
        break;
    case ThorQ::Serialization::Body_moderation:
        handleMessageModeration(context);
        break;
    case ThorQ::Serialization::Body_system_id:
        handleMessageSystemID(context);
        break;
    case ThorQ::Serialization::Body_user:
        handleMessageUser(context);
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(context);
        break;
    case ThorQ::Serialization::Body_p2p:
        handleMessageP2P(context);
        break;
    default:
        fmt::print("[MSG] Invalid\n");
        m_connection->disconnect();
        return;
    }
}

bool ThorQ::ApiClient::sendContextData(MessageContext &context)
{
    auto fbsRespBuffer = ThorQ::Serialization::CreateMessageBufferDirect(context.fbsBuilder, &context.messages);
    context.fbsBuilder.Finish(fbsRespBuffer);

    bool result = encodeAndSend(context.fbsBuilder.GetBufferSpan(), context.encrypt);

    context.messages.clear();
    context.fbsBuilder.Clear();
    context.encrypt = true;

    return result;
}

void ThorQ::ApiClient::handleMessageAnnouncement(MessageContext& context)
{
    auto fbsAnnouncement = static_cast<const ThorQ::Serialization::Announcement::Message*>(context.body);

    fmt::print("[MSG] Announcement\n");
}

void ThorQ::ApiClient::handleMessageDevice(MessageContext& context)
{
    auto fbsDevice = static_cast<const ThorQ::Serialization::Device::Message*>(context.body);

    fmt::print("[MSG] Device\n");
}

void ThorQ::ApiClient::handleMessageCrypto(MessageContext& context)
{
    auto fbsServerKey = static_cast<const ThorQ::Serialization::Crypto::Message*>(context.body)->body_as_server_key();

    fmt::print("[MSG] Crypto\n");

    if (fbsServerKey == nullptr) {
        fmt::print(stderr, "[CRYPTO] Got nullptr!\n");
        disconnect();
        return;
    }

    ThorQ::Crypto::Signer serverVerifier;
    serverVerifier.setPublicKey(ThorQ::Crypto::Signer::RootSigner());

    const auto& clientPublicKey = m_crypto.publicKey();
    const auto& serverPublicKey = *fbsServerKey->public_key();
    const auto& serverSignature = *fbsServerKey->signature();

    std::array<std::uint8_t, ThorQ::Crypto::Encryption::PublicKeyLen * 2> combinedPublicKeys;

    auto pk1 = combinedPublicKeys.data();
    auto pk2 = pk1 + ThorQ::Crypto::Encryption::PublicKeyLen;

    std::memcpy(pk1, clientPublicKey.data(), ThorQ::Crypto::Encryption::PublicKeyLen);
    std::memcpy(pk2, serverPublicKey.Data(), ThorQ::Crypto::Encryption::PublicKeyLen);

    if (!serverVerifier.verify(combinedPublicKeys, fromFbsArray<std::uint8_t, 64>(serverSignature)))
    {
        fmt::print(stderr, "[CRYPTO] Failed to verify server key validity!\n");
        m_connection->disconnect();
        return;
    }

    // Set foreign key
    m_crypto.setForeignKey(fromFbsArray<std::uint8_t, 32>(serverPublicKey));

    onCryptoEstablished(context);
}

void ThorQ::ApiClient::handleMessageFile(MessageContext& context)
{
    auto fbsCrypto = static_cast<const ThorQ::Serialization::File::Message*>(context.body);

    fmt::print("[MSG] File\n");
}

void ThorQ::ApiClient::handleMessageFriendRequest(MessageContext& context)
{
    auto fbsCrypto = static_cast<const ThorQ::Serialization::FriendRequest::Message*>(context.body);

    fmt::print("[MSG] Friend request\n");
}

void ThorQ::ApiClient::handleMessageGroup(MessageContext& context)
{
    auto fbsCrypto = static_cast<const ThorQ::Serialization::Group::Message*>(context.body);

    fmt::print("[MSG] Group\n");
}

void ThorQ::ApiClient::handleMessageModeration(MessageContext& context)
{
    auto fbsCrypto = static_cast<const ThorQ::Serialization::Moderation::Message*>(context.body);

    fmt::print("[MSG] Moderation\n");
}

void ThorQ::ApiClient::handleMessageSystemID(MessageContext& context)
{
    auto fbsCrypto = static_cast<const ThorQ::Serialization::SystemId::Message*>(context.body);

    fmt::print("[MSG] Systemid\n");
}

void ThorQ::ApiClient::handleMessageUser(MessageContext& context)
{
    auto fbsCrypto = static_cast<const ThorQ::Serialization::User::Message*>(context.body);

    fmt::print("[MSG] User\n");
}

void ThorQ::ApiClient::handleMessageVersion(MessageContext& context)
{
    auto fbsVersion = static_cast<const ThorQ::Serialization::Version*>(context.body);

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
            m_connection->disconnect();
        }
        break;
    case THORQ_APP::CLIENT:
        if (version == ThorQ::ClientVersion) {
            fmt::print("[VERSION] Client version matched\n");
        } else {
            fmt::print("[VERSION] Client version mismatched\n");
            m_connection->disconnect();
        }
        break;
    case THORQ_APP::LINK:
        if (version == ThorQ::LinkVersion) {
            fmt::print("[VERSION] Link version matched\n");
        } else {
            fmt::print("[VERSION] Link version mismatched\n");
            m_connection->disconnect();
        }
        break;
    default:
        fmt::print("[VERSION] invalid\n");
        m_connection->disconnect();
        return;
    }
}

void ThorQ::ApiClient::handleMessageP2P(MessageContext& context)
{
    auto fbsP2P = static_cast<const ThorQ::Serialization::Peer2Peer::Message*>(context.body);
}

bool ThorQ::ApiClient::encodeAndSend(const std::span<std::uint8_t>& buffer, bool encrypt)
{
    auto message = std::make_shared<std::vector<std::uint8_t>>();
    message->resize(ThorQ::Encoding::calculateMessageSize(buffer.size(), encrypt));

    if (encrypt) {
        if (!ThorQ::Encoding::messageEncode(buffer.data(), buffer.size(), message->data(), message->size(), m_crypto)) {
            fmt::print(stderr, "Failed to encrypt message\n");
            return false;
        }
    }
    else {
        if (!ThorQ::Encoding::messageEncode(buffer.data(), buffer.size(), message->data(), message->size())) {
            fmt::print(stderr, "Failed to encode message\n");
            return false;
        }
    }

    auto connection = m_connection;
    if (connection != nullptr) {
        connection->messageSend(message);
    }

    return true;
}
