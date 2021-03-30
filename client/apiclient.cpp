#include "apiclient.h"

#include "accountcontroller.h"
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
        fmt::print("POLL\n");
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
    establishCrypto();
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

    HandlerContext context;

    // Message will be encrypted by default
    context.encrypt = true;

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

void ThorQ::ApiClient::establishCrypto()
{
    fmt::print("EstablishCrypto\n");

    if (!m_crypto.generateKeyPair()) {
        fmt::print(stderr, "[CRYPTO] Failed to generate keypair!\n");
        m_connection->disconnect();
        return;
    }

    auto myPk = m_crypto.publicKey();

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsPublicKey = fbsBuilder.CreateVector(myPk.data(), myPk.size());
    auto fbsCrypto    = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, fbsPublicKey).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsCrypto));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    encodeAndSend(fbsBuilder.GetBufferSpan(), false);
}

void ThorQ::ApiClient::onCryptoEstablished()
{
    fmt::print("[CONNECTION] Crypto established!\n");

    std::vector<std::uint8_t> systemID = ThorQ::SystemID::systemid_generate();
    fmt::print("Sending SystemID: {}\n", ThorQ::SystemID::systemid_to_string(systemID));

    flatbuffers::FlatBufferBuilder fbsBuilder;
    flatbuffers::Offset<ThorQ::Serialization::Version> fbsVersion;
    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> fbsMessageVector;

    // Link version
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::LINK, THORQ_VERSION_LINK_MAJOR, THORQ_VERSION_LINK_MINOR, THORQ_VERSION_LINK_PATCH);
    fbsMessageVector.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union()));

    // Client version
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::CLIENT, THORQ_VERSION_CLIENT_MAJOR, THORQ_VERSION_CLIENT_MINOR, THORQ_VERSION_CLIENT_PATCH);
    fbsMessageVector.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union()));

    // Server version
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::SERVER, THORQ_VERSION_SERVER_MAJOR, THORQ_VERSION_SERVER_MINOR, THORQ_VERSION_SERVER_PATCH);
    fbsMessageVector.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union()));

    // Hardware ID
    auto fbsSystemID = ThorQ::Serialization::SystemId::CreateMessage(fbsBuilder, ThorQ::Serialization::SystemId::Command_Submit, fbsBuilder.CreateVector(systemID.data(), systemID.size())).Union();
    fbsMessageVector.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_system_id, fbsSystemID));

    auto fbsMessageBuffer = ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &fbsMessageVector);

    fbsBuilder.Finish(fbsMessageBuffer);
    encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::ApiClient::handleMessage(HandlerContext& context)
{
    auto fbsMessage = reinterpret_cast<const ThorQ::Serialization::Message*>(context.body);

    context.body = fbsMessage->body();
    if (fbsMessage == nullptr) {
        fmt::print("[MSG] null\n");
        disconnect();
        return;
    }

    context.requestId = fbsMessage->request_id();

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

bool ThorQ::ApiClient::sendContextData(HandlerContext &context)
{
    auto fbsRespBuffer = ThorQ::Serialization::CreateMessageBufferDirect(context.fbsBuilder, &context.messages);
    context.fbsBuilder.Finish(fbsRespBuffer);

    bool result = encodeAndSend(context.fbsBuilder.GetBufferSpan(), context.encrypt);

    context.messages.clear();
    context.fbsBuilder.Clear();
    context.encrypt = true;

    return result;
}

void ThorQ::ApiClient::handleMessageAnnouncement(HandlerContext& context)
{
    auto fbsAnnouncement = reinterpret_cast<const ThorQ::Serialization::Announcement::Message*>(context.body);

    fmt::print("[MSG] Announcement\n");
}

void ThorQ::ApiClient::handleMessageDevice(HandlerContext& context)
{
    auto fbsDevice = reinterpret_cast<const ThorQ::Serialization::Device::Message*>(context.body);

    fmt::print("[MSG] Device\n");
}

void ThorQ::ApiClient::handleMessageCrypto(HandlerContext& context)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(context.body);

    fmt::print("[MSG] Crypto\n");

    auto fbsServerPublicKey = fbsCrypto->public_key();
    auto fbsServerSignature = fbsCrypto->signature();

    if (fbsServerPublicKey->size() != ThorQ::Crypto::Encryption::PublicKeyLen ||
        fbsServerSignature->size() != ThorQ::Crypto::Signer::SignatureLen)
    {
        fmt::print(stderr, "[CRYPTO] Got key of invalid size!\n");
        m_connection->disconnect();
        return;
    }

    ThorQ::Crypto::Signer serverVerifier;
    serverVerifier.setPublicKey(ThorQ::Crypto::Signer::RootSigner());

    std::array<std::uint8_t, ThorQ::Crypto::Signer::PublicKeyLen * 2> combinedPublicKeys;
    memcpy(combinedPublicKeys.data(), m_crypto.publicKey().data(), ThorQ::Crypto::Encryption::PublicKeyLen);
    memcpy(combinedPublicKeys.data() + ThorQ::Crypto::Encryption::PublicKeyLen, fbsServerPublicKey->data(), ThorQ::Crypto::Encryption::PublicKeyLen);

    if (!serverVerifier.verify(combinedPublicKeys, fbsServerSignature->data(), fbsServerSignature->size()))
    {
        fmt::print(stderr, "[CRYPTO] Failed to verify server key validity!\n");
        m_connection->disconnect();
        return;
    }

    // Set foreign key
    if (!m_crypto.setForeignKey(fbsServerPublicKey->data(), fbsServerPublicKey->size()))
    {
        fmt::print(stderr, "[CRYPTO] Failed to set foreignkey!\n");
        m_connection->disconnect();
        return;
    }

    onCryptoEstablished();
}

void ThorQ::ApiClient::handleMessageFile(HandlerContext& context)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::File::Message*>(context.body);

    fmt::print("[MSG] File\n");
}

void ThorQ::ApiClient::handleMessageFriendRequest(HandlerContext& context)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::FriendRequest::Message*>(context.body);

    fmt::print("[MSG] Friend request\n");
}

void ThorQ::ApiClient::handleMessageGroup(HandlerContext& context)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Group::Message*>(context.body);

    fmt::print("[MSG] Group\n");
}

void ThorQ::ApiClient::handleMessageModeration(HandlerContext& context)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Moderation::Message*>(context.body);

    fmt::print("[MSG] Moderation\n");
}

void ThorQ::ApiClient::handleMessageSystemID(HandlerContext& context)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::SystemId::Message*>(context.body);

    fmt::print("[MSG] Systemid\n");
}

void ThorQ::ApiClient::handleMessageUser(HandlerContext& context)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::User::Message*>(context.body);

    fmt::print("[MSG] User\n");
}

void ThorQ::ApiClient::handleMessageVersion(HandlerContext& context)
{
    auto fbsVersion = reinterpret_cast<const ThorQ::Serialization::Version*>(context.body);

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

void ThorQ::ApiClient::handleMessageP2P(HandlerContext& context)
{
    auto fbsP2P = reinterpret_cast<const ThorQ::Serialization::Peer2Peer::Message*>(context.body);
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
