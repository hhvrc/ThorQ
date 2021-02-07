#include "apiserver_connection.h"

#include <cryptography/signer.h>
#include <networking/tcpconnection.h>

#include <systemid.h>
#include <encoding.h>
#include <enums.h>
#include <utils.h>

#include <schemas_common.h>

#include <lsql/transaction.h>
#include <lsql/connection.h>
#include <lsql/column.h>
#include <lsql/query.h>
#include <fmt/core.h>

ThorQ::ApiServerConnection::ApiServerConnection(asio::io_context& asio, asio::ip::tcp::socket socket)
    : ThorQ::Networking::TcpConnection(asio, std::move(socket))
    , m_buffer(THORQ_PAYLOAD_LEN_TYP)
    , m_crypto()
    , l_account()
    , m_account(nullptr)
    , l_systemID()
    , m_systemID(nullptr)
{
}

ThorQ::ApiServerConnection::ApiServerConnection(ThorQ::ApiServerConnection&& other)
    : ThorQ::Networking::TcpConnection(std::move(other))
    , m_buffer(std::move(other.m_buffer))
    , m_crypto(std::move(other.m_crypto))
    , l_account()
    , m_account(std::move(other.m_account))
    , l_systemID()
    , m_systemID(std::move(other.m_systemID))
{

}

ThorQ::ApiServerConnection::~ApiServerConnection()
{
    disconnect();
    setAccount(nullptr);
    setSystemID(nullptr);
}

std::shared_ptr<ThorQ::Account> ThorQ::ApiServerConnection::account() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_account));
    return m_account;
}

void ThorQ::ApiServerConnection::setAccount(std::shared_ptr<ThorQ::Account> account)
{
    std::unique_lock l(l_account);
    m_account = account;
}

std::shared_ptr<std::vector<uint8_t> > ThorQ::ApiServerConnection::systemID() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_systemID));
    return m_systemID;
}

void ThorQ::ApiServerConnection::setSystemID(std::shared_ptr<std::vector<uint8_t> > systemID)
{
    std::unique_lock l(l_systemID);
    m_systemID = systemID;
}

void ThorQ::ApiServerConnection::onError(std::error_code ec)
{
    fmt::print(stderr, "[CONNECTION] Error: {}\n", ec.message());
}

void ThorQ::ApiServerConnection::onConnect(std::vector<std::uint8_t> address, std::uint16_t port)
{
    fmt::print("[CONNECTION] Connected\n");

    flatbuffers::FlatBufferBuilder fbsBuilder;
    flatbuffers::Offset<ThorQ::Serialization::Version> fbsVersion;
    flatbuffers::Offset<ThorQ::Serialization::Message> fbsMessage;

    // Link version
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::LINK, THORQ_VERSION_LINK_MAJOR, THORQ_VERSION_LINK_MINOR, THORQ_VERSION_LINK_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    encodeAndSend(fbsBuilder.GetBufferSpan(), false);

    // Client version
    fbsBuilder.Clear();
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::CLIENT, THORQ_VERSION_CLIENT_MAJOR, THORQ_VERSION_CLIENT_MINOR, THORQ_VERSION_CLIENT_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    encodeAndSend(fbsBuilder.GetBufferSpan(), false);

    // Server version
    fbsBuilder.Clear();
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::SERVER, THORQ_VERSION_SERVER_MAJOR, THORQ_VERSION_SERVER_MINOR, THORQ_VERSION_SERVER_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    encodeAndSend(fbsBuilder.GetBufferSpan(), false);
}

void ThorQ::ApiServerConnection::onDisconnect()
{
    fmt::print("[CONNECTION] Disconnected\n");
}

bool ThorQ::ApiServerConnection::onHeader(const ThorQ::Encoding::MessageHeader* header)
{
    fmt::print("[CONNECTION] Header {}\n", ntohl(header->bodySize));

    return ThorQ::Encoding::isHeaderValid(header);
}

void ThorQ::ApiServerConnection::onMessage(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    fmt::print("[CONNECTION] Message\n");

    if (!ThorQ::Encoding::isMessageValid(message->data(), message->size()))
    {
        fmt::print(stderr, "Received packet is corrupted\n");
        disconnect();
        return;
    }

    m_buffer.resize(ThorQ::Encoding::calculateDataSize(message->data(), message->size()));
    if (!ThorQ::Encoding::messageDecode(message->data(), message->size(), m_buffer.data(), m_buffer.size(), m_crypto))
    {
        fmt::print(stderr, "Cannot decode/decrypt packet\n");
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
        disconnect();
        return;
    }
}

void ThorQ::ApiServerConnection::onCryptoEstablished()
{
}

void ThorQ::ApiServerConnection::handleMessageAccount(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::Account::Message*>(body);

    if (!fbsAccount->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] Account\n");

    switch (fbsAccount->body_type())
    {
    case ThorQ::Serialization::Account::Body_get_account:
        break;
    case ThorQ::Serialization::Account::Body_account:
        break;
    case ThorQ::Serialization::Account::Body_get_auth_token:
        break;
    case ThorQ::Serialization::Account::Body_login:
        break;
    case ThorQ::Serialization::Account::Body_register_:
        break;
    case ThorQ::Serialization::Account::Body_recover:
        break;
    case ThorQ::Serialization::Account::Body_delete_:
        break;
    case ThorQ::Serialization::Account::Body_logout:
        break;
    case ThorQ::Serialization::Account::Body_generate_seed:
        break;
    case ThorQ::Serialization::Account::Body_seed_generated:
        break;
    case ThorQ::Serialization::Account::Body_set_username:
        break;
    case ThorQ::Serialization::Account::Body_set_password:
        break;
    case ThorQ::Serialization::Account::Body_set_email:
        break;
    case ThorQ::Serialization::Account::Body_set_image:
        break;
    default:
        break;
    }/*
    std::string username, password;
    thorq_payload_login_get_username(message, username);
    thorq_payload_login_get_password(message, password);

    if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDOUT)
    {
        auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [&](const std::shared_ptr<ThorQ::Account> account) -> bool
        {
            return account->username() == username;
        });

        if (it != g_accounts.end())
        {
            qDebug() << username << "logged in";

            instance->setAccount(*it);
            instance->setLoginState(THORQ_STATE_LOGIN_LOGGEDIN);

            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_OK);
            instance->packetSend(response, true, true);
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_DENIED);

            instance->packetSend(response, true, true);
        }
    }
    else
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_NO_CHANGE);
        instance->packetSend(response, true, true);
    }*/
}

void ThorQ::ApiServerConnection::handleMessageAnnouncement(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsAnnouncement = reinterpret_cast<const ThorQ::Serialization::Announcement::Message*>(body);

    if (!fbsAnnouncement->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] Announcement\n");
}

void ThorQ::ApiServerConnection::handleMessageDevice(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsDevice = reinterpret_cast<const ThorQ::Serialization::Device::Message*>(body);

    if (!fbsDevice->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] Device\n");

    /*
    if (instance->sessionState() == THORQ_STATE_SESSION_ACTIVE)
        if (instance->partner() != nullptr)
            instance->partner()->sendMessage(message, true, false);
    */
}

void ThorQ::ApiServerConnection::handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(body);

    if (!fbsCrypto->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] Crypto\n");

    auto fbsClientPublicKey = fbsCrypto->public_key();

    if (!m_crypto.generateKeyPair()) {
        fmt::print(stderr, "[CRYPTO] Failed to generate keypair!\n");
        return;
    }

    if (m_crypto.setForeignKey(fbsClientPublicKey->data(), fbsClientPublicKey->size()))
    {
        auto serverPublickey = m_crypto.publicKey();

        ThorQ::Crypto::Signer signer;
        signer.generateKeyPair();

        // TODO: load this at server startup
        if (!signer.tryLoadFromFile("root_signing.pksk")) {
            fmt::print(stderr, "[CRYPTO] Failed to load signer keypair!\n");
            return;
        }

        // PUBLIC_CLIENT_KEY + PUBLIC_SERVER_KEY
        std::array<std::uint8_t, ThorQ::Crypto::Signer::PublicKeyLen * 2> combinedPublicKeys;
        memcpy(combinedPublicKeys.data(), fbsClientPublicKey->data(), ThorQ::Crypto::Encryption::PublicKeyLen);
        memcpy(combinedPublicKeys.data() + ThorQ::Crypto::Encryption::PublicKeyLen, serverPublickey.data(), ThorQ::Crypto::Encryption::PublicKeyLen);

        // SIGNATURE(PUBLIC_CLIENT_KEY + PUBLIC_SERVER_KEY)
        std::array<std::uint8_t, ThorQ::Crypto::Signer::SignatureLen> signature;
        if (!signer.sign(combinedPublicKeys, signature)) {
            fmt::print(stderr, "[CRYPTO] Failed to sign encryption public key!\n");
            return;
        }

        // Build flatbuffer
        flatbuffers::FlatBufferBuilder fbsBuilder;
        auto fbsPublicKey = fbsBuilder.CreateVector(serverPublickey.data(), serverPublickey.size());
        auto fbsSignature = fbsBuilder.CreateVector(signature.data(), signature.size());
        auto fbsCrypto    = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, fbsPublicKey, fbsSignature).Union();
        auto fbsMessage   = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsCrypto);
        fbsBuilder.Finish(fbsMessage);

        if (encodeAndSend(fbsBuilder.GetBufferSpan(), false)) {
            onCryptoEstablished();
        }
    }
    else
    {
        fmt::print(stderr, "[CRYPTO] Got key with invalid length!\n");
        disconnect(); // TODO: In the future should find a way to send reason for disconnet to server as well
    }
}

void ThorQ::ApiServerConnection::handleMessageFile(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsFile = reinterpret_cast<const ThorQ::Serialization::File::Message*>(body);

    if (!fbsFile->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] File\n");
}

void ThorQ::ApiServerConnection::handleMessageFriendRequest(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsFriendRequest = reinterpret_cast<const ThorQ::Serialization::FriendRequest::Message*>(body);

    if (!fbsFriendRequest->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] Friend request\n");

    /*
    std::vector<std::uint8_t> response;

    THORQ_COMMAND_ID cmd;
    thorq_payload_command_get_id(message, cmd);

    if (instance->authState() != THORQ_STATE_AUTH_OK)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_UNAUTHORIZED);
        instance->packetSend(response, false, true);
        return;
    }

    switch (cmd){
    case THORQ_COMMAND_ID_GET_USER_LIST:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_OK);
            instance->packetSend(response, true, true);

            std::vector<ThorQ::Instance*> instances = g_sessions.toList();

            for (ThorQ::Instance* i : instances)
            {
                if (i->account() != nullptr)
                {
                    thorq_payload_notification_pack(response, THORQ_NOTIFICATION_USER_ACTIVITY, i->account()->username(), i->activityState());
                    instance->packetSend(response, true, true);
                }
            }
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_REQUEST:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string username;
            thorq_payload_command_get_data(message, username);

            auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [&](const std::shared_ptr<ThorQ::Account> account) -> bool
            {
                return account->username() == username;
            });

            if (*it == nullptr)
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_PAYLOAD_ACK_DENIED, username + "is not an account");
                instance->packetSend(response, true);
                return;
            }

            std::unordered_setThorQ::Instance*> targetInstances = (*it)->instances();

            if (targetInstances.isEmpty())
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_PAYLOAD_ACK_DENIED, username + "is not online");
                instance->packetSend(response, true);
                return;
            }

            for (ThorQ::Instance* otherInstance : (*it)->instances())
            instance->requestOn(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_ACCEPT:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string name;
            thorq_payload_command_get_data(message, name);

            ThorQ::Instance* otherInstance = g_sessions->get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_DENIED, name + "is not online");
                instance->packetSend(response, true);
                return;
            }

            instance->requestAcceptFrom(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_PAYLOAD_ROOM_
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string name;
            thorq_payload_
            thorq_payload_command_get_data(message, name);

            auto sit = std::find_if(g_accounts.begin(), g_accounts.end(), [name](const std::shared_ptr<ThorQ::Account> a) -> bool
            {
                if (a == nullptr) return false;

                return a->username() == name;
            });
            ThorQ::Instance* otherInstance = g_sessions .get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_DENIED, name + "is not online");
                instance->packetSend(response, true);
                return;
            }

            instance->requestDenyFrom(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_LEAVE:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            instance->setSessionState(THORQ_STATE_SESSION_NONE);

            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_OK);
            instance->packetSend(response, true, true);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SET_SELF_STATE:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::uint8_t state;
            thorq_payload_command_get_data(message, state);
            instance->setActivityState(state);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    }
    */
}

void ThorQ::ApiServerConnection::handleMessageGroup(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsGroup = reinterpret_cast<const ThorQ::Serialization::Group::Message*>(body);

    if (!fbsGroup->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] Group\n");

    /*
    std::vector<std::uint8_t> response;

    if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
    {
        instance->setLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);

        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGOUT, THORQ_PAYLOAD_ACK_OK);
        instance->packetSend(response, true, true);
    }
    else
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGOUT, THORQ_PAYLOAD_ACK_NO_CHANGE);
        instance->packetSend(response, true, true);
    }
    */
}

void ThorQ::ApiServerConnection::handleMessageModeration(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsModeration = reinterpret_cast<const ThorQ::Serialization::Moderation::Message*>(body);

    if (!fbsModeration->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] Moderation\n");
}

void ThorQ::ApiServerConnection::handleMessageSystemID(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsSystemID = reinterpret_cast<const ThorQ::Serialization::SystemId::Message*>(body);

    if (!fbsSystemID->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] SystemID\n");

    if (fbsSystemID->cmd() != ThorQ::Serialization::SystemId::Command_Submit)
    {
        // TODO: THORQ_DISCONNECT_REASON::INVALID_OPERATION
        disconnect();
        return;
    }

    std::span<std::uint8_t> systemid(const_cast<std::uint8_t*>(fbsSystemID->data()->data()), fbsSystemID->data()->size());

    if (!ThorQ::SystemID::systemid_validate(systemid))
    {
        // TODO: THORQ_DISCONNECT_REASON::INVALID_HWID
        disconnect();
        return;
    }

    std::string systemID = ThorQ::SystemID::systemid_to_string(systemid);

    fmt::print("SystemID: {}\n", systemID);

    LSql::Connection dbConnection("database.db", LSql::Connection::READWRITE);

    if (!dbConnection.isOpen())
    {
        return;
    }

    LSql::Query dbQuery = dbConnection.query("INSERT OR IGNORE INTO system_ids(system_id) VALUES (?1);"
                                             "SELECT banned_at FROM system_ids WHERE system_id = ?1;");
    dbQuery.bind(1, systemID);

    if (!dbQuery.step())
    {
        fmt::print(stderr, "Failed to execute SQLite query!\n");
        return;
    }

    if (dbQuery.columnCount() == 0)
    {
        fmt::print(stderr, "SQLite query returned invalid amount of rows ({})\n", dbQuery.columnCount());
        return;
    }

    bool isBanned = (dbQuery.column(0).type() != LSql::Type::Null);

    if (isBanned)
    {
        fmt::print("Connection is banned!\n");
        disconnect();
        return;
    }
}

void ThorQ::ApiServerConnection::handleMessageUser(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsUser = reinterpret_cast<const ThorQ::Serialization::User::Message*>(body);

    if (!fbsUser->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] User\n");
}

void ThorQ::ApiServerConnection::handleMessageVersion(const void* body, flatbuffers::Verifier fbsVerifier)
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

void ThorQ::ApiServerConnection::handleMessageP2P(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsP2P = reinterpret_cast<const ThorQ::Serialization::Peer2Peer::Message*>(body);

    if (!fbsP2P->Verify(fbsVerifier)) {
        return;
    }

    fmt::print("[MSG] P2P\n");
}

bool ThorQ::ApiServerConnection::encodeAndSend(flatbuffers::span<std::uint8_t> buffer, bool encrypt)
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

    messageSend(message);
}
