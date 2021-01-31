#include "apiconnectionhandler.h"

#include <networking/message.h>
#include <encoding.h>
#include <cryptography/encryption.h>

ThorQ::ApiConnectionHandler::ApiConnectionHandler()
{
}

ThorQ::ApiConnectionHandler::~ApiConnectionHandler()
{
    setCrypto(nullptr);
    setAccount(nullptr);
    setSystemID(nullptr);
}

std::shared_ptr<ThorQ::Crypto::Encryption> ThorQ::ApiConnectionHandler::crypto() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_crypto));
    return m_crypto;
}

void ThorQ::ApiConnectionHandler::setCrypto(std::shared_ptr<ThorQ::Crypto::Encryption> crypto)
{
    std::unique_lock l(l_crypto);
    m_crypto = crypto;
}

std::shared_ptr<ThorQ::Account> ThorQ::ApiConnectionHandler::account() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_account));
    return m_account;
}

void ThorQ::ApiConnectionHandler::setAccount(std::shared_ptr<ThorQ::Account> account)
{
    std::unique_lock l(l_account);
    m_account = account;
}

std::shared_ptr<std::vector<uint8_t> > ThorQ::ApiConnectionHandler::systemID() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_systemID));
    return m_systemID;
}

void ThorQ::ApiConnectionHandler::setSystemID(std::shared_ptr<std::vector<uint8_t> > systemID)
{
    std::unique_lock l(l_systemID);
    m_systemID = systemID;
}

void ThorQ::ApiConnectionHandler::onConnect()
{

}

void ThorQ::ApiConnectionHandler::onDisconnect()
{

}

bool ThorQ::ApiConnectionHandler::onHeader(std::shared_ptr<ThorQ::Networking::MessageHeader> header)
{
    if (header->size > ThorQ::Encoding::calculateEncodedSize(THORQ_PAYLOAD_LEN_MAX, true) || header->size < THORQ_PAYLOAD_LEN_MIN)
    {
        return false;
    }
    return true;
}

void ThorQ::ApiConnectionHandler::onMessage(std::shared_ptr<std::vector<std::uint8_t> > message)
{

}


#if 0
void ThorQ::MessageHandler::handleEventConnection(const ENetEvent& event)
{
    // Dont worry, its ok to have a seemingly dangling pointer here (ENet keeps track of the pointer)

    ThorQ::ApiConnection* instance = new ThorQ::ApiConnection(event.peer);

    flatbuffers::FlatBufferBuilder fbsBuilder;
    flatbuffers::Offset<ThorQ::Serialization::Version> fbsVersion;
    flatbuffers::Offset<ThorQ::Serialization::Message> fbsMessage;

    // Link version
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::LINK, THORQ_VERSION_LINK_MAJOR, THORQ_VERSION_LINK_MINOR, THORQ_VERSION_LINK_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    sendPacket(instance., fbsBuilder.GetBufferSpan(), false);

    // Client version
    fbsBuilder.Clear();
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::CLIENT, THORQ_VERSION_CLIENT_MAJOR, THORQ_VERSION_CLIENT_MINOR, THORQ_VERSION_CLIENT_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    sendPacket(instance, fbsBuilder.GetBufferSpan(), false);

    // Server version
    fbsBuilder.Clear();
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::SERVER, THORQ_VERSION_SERVER_MAJOR, THORQ_VERSION_SERVER_MINOR, THORQ_VERSION_SERVER_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    sendPacket(instance, fbsBuilder.GetBufferSpan(), false);

    char addr[40];
    if (enet_peer_get_ip(event.peer, addr, 40) == 0)
    {
        fmt::print("[{}] Connected\n", addr);
    }
}
void ThorQ::MessageHandler::handleEventMessage(const ENetEvent& event)
{
    if (event.peer == nullptr || event.peer->data == nullptr ||
        event.channelID > (std::uint8_t)THORQ_CHANNEL::_MAX  ||
        !ThorQ::checkDataSize(event.packet))
    {
        return;
    }

    ThorQ::ApiConnection* instance = reinterpret_cast<ThorQ::ApiConnection*>(event.peer->data);

    m_tempBuffer.resize(ThorQ::calculateDataSize(event.packet));
    if (!ThorQ::dataDecode(event.packet, m_tempBuffer, instance->m_crypto))
    {
        return;
    }

    auto fbsMessage = flatbuffers::GetRoot<ThorQ::Serialization::Message>(m_tempBuffer.data());
    auto fbsVerifier = flatbuffers::Verifier(m_tempBuffer.data(), m_tempBuffer.size());

    if (!fbsMessage->Verify(fbsVerifier))
    {
        return;
    }

    switch ((THORQ_CHANNEL)event.channelID) {
    case THORQ_CHANNEL::API:
        break;
    case THORQ_CHANNEL::EVENTS:
        break;
    case THORQ_CHANNEL::RTC:
        break;
    case THORQ_CHANNEL::AUTHORITY:
        break;
    case THORQ_CHANNEL::_MAX:
    case THORQ_CHANNEL::_INVALID:
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        onMessageAccount(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_version:
        onMessageVersion(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_crypto:
        onMessageCrypto(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_system_id:
        onMessageSystemID(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_group:
        onMessageGroup(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_device:
        onMessageDevice(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_moderation:
        onMessageModeration(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_friend_request:
        onMessageFriendRequest(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_file:
        onMessageFile(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_user:
        onMessageUser(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_announcement:
        fmt::print("Unexpected messageID from client: {}\n", m_tempBuffer[0]);
        break;
    case ThorQ::Serialization::Body_NONE:
    default:
        return;
    }
}
void ThorQ::MessageHandler::handleEventDisconnect(const ENetEvent& event)
{
    // Get instance
    ThorQ::ApiConnection* instance = reinterpret_cast<ThorQ::ApiConnection*>(event.peer->data);

    // Yeet
    delete instance;

    char addr[40];
    if (enet_peer_get_ip(event.peer, addr, 40) == 0)
    {
        fmt::print("[{}] disconnected\n", addr);
    }
}
void ThorQ::MessageHandler::handleEventTimeout(const ENetEvent &event)
{
    char addr[40];
    if (enet_peer_get_ip(event.peer, addr, 40) == 0)
    {
        fmt::print("[{}] timed out\n", addr);
    }
}

void ThorQ::MessageHandler::onMessageAccount(ThorQ::ApiConnection *instance, const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::Account::Message*>(body);

    if (!fbsAccount->Verify(fbsVerifier))
    {
        return;
    }

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

void ThorQ::MessageHandler::onMessageDevice(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsDevice = reinterpret_cast<const ThorQ::Serialization::Device::Message*>(body);

    if (!fbsDevice->Verify(fbsVerifier))
    {
        return;
    }

    /*
    if (instance->sessionState() == THORQ_STATE_SESSION_ACTIVE)
        if (instance->partner() != nullptr)
            instance->partner()->sendMessage(message, true, false);
    */
}

void ThorQ::MessageHandler::onMessageCrypto(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(body);

    if (!fbsCrypto->Verify(fbsVerifier))
    {
        return;
    }

    fmt::print("[MSG] Crypto!\n");

    switch (fbsCrypto->type()) {
    case ThorQ::Serialization::Crypto::MessageType_Request:
    {
        fmt::print("[MSG] Crypto request!\n");
        instance->m_crypto.generateKeyPair();

        std::array<std::uint8_t, ThorQ::Crypto::Encryption::PublicKeyLen> pubKey;
        instance->m_crypto.getPublicKey(pubKey);

        // Build flatbuffer
        flatbuffers::FlatBufferBuilder fbsBuilder;
        auto fbsEstablish     = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Establish, fbsBuilder.CreateVector(pubKey.data(), pubKey.size())).Union();
        auto fbsMessage       = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsEstablish);
        fbsBuilder.Finish(fbsMessage);

        sendPacket(instance, fbsBuilder.GetBufferSpan(), false);
        break;
    }
    case ThorQ::Serialization::Crypto::MessageType_Establish:
    {
        fmt::print("[MSG] Crypto establish!\n");

        if (fbsCrypto->data()->size() != ThorQ::Crypto::Encryption::PublicKeyLen)
        {
            fmt::print("Got key with invalid length!\n");
            return;
        }

        std::span<std::uint8_t, ThorQ::Crypto::Encryption::PublicKeyLen> data(
                        const_cast<std::uint8_t*>(fbsCrypto->data()->data()),
                        fbsCrypto->data()->size()
                    );

        if (instance->m_crypto.agreeAsServer(data))
        {
            ThorQ::Crypto::Encryption::RandomizeBytes(instance->m_verificationData);

            // Build flatbuffer
            flatbuffers::FlatBufferBuilder fbsBuilder;
            auto fbsVerify  = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Verify, fbsBuilder.CreateVector(instance->m_verificationData.data(), instance->m_verificationData.size())).Union();
            auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsVerify);
            fbsBuilder.Finish(fbsMessage);

            sendPacket(instance, fbsBuilder.GetBufferSpan(), true);
        }
        else
        {
            char buf[50];
            enet_peer_get_ip(instance->m_peer, buf, 50);
            fmt::print(stderr, "Failed to create shared secret with {}\n", buf);
            m_server->tryQueueDisconnect(instance->m_peer, false, THORQ_DISCONNECT_REASON::CRYPTO_FAILED, m_tokenQueue);
        }
        break;
    }
    case ThorQ::Serialization::Crypto::MessageType_Verify:
    {
        fmt::print("[MSG] Crypto verify!\n");

        if (fbsCrypto->data()->size() == instance->m_verificationData.size() &&
            memcmp(fbsCrypto->data()->data(), instance->m_verificationData.data(), instance->m_verificationData.size()) == 0)
        {
            fmt::print("[MSG] Crypto verified!\n");

            // Build flatbuffer
            flatbuffers::FlatBufferBuilder fbsBuilder;
            auto fbsVerify  = ThorQ::Serialization::Crypto::CreateMessageDirect(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Acknowledge).Union();
            auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsVerify);
            fbsBuilder.Finish(fbsMessage);

            sendPacket(instance, fbsBuilder.GetBufferSpan(), true);
        }
        else
        {
            char buf[50];
            enet_peer_get_ip(instance->m_peer, buf, 50);

            fmt::print(stderr, "Failed to verify with {}\n", buf);
            m_server->tryQueueDisconnect(instance->m_peer, false, THORQ_DISCONNECT_REASON::CRYPTO_FAILED, m_tokenQueue);
        }
        break;
    }
    default:
        fmt::print("[MSG] Crypto \?\?\?!\n");
        return;
    }
}

void ThorQ::MessageHandler::onMessageFile(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsFile = reinterpret_cast<const ThorQ::Serialization::File::Message*>(body);

    if (!fbsFile->Verify(fbsVerifier))
    {
        return;
    }
}
#include <flatbuffers/flexbuffers.h>
void ThorQ::MessageHandler::onMessageFriendRequest(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::FriendRequest::Message*>(body);

    if (!fbsAccount->Verify(fbsVerifier))
    {
        return;
    }

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

void ThorQ::MessageHandler::onMessageGroup(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsGroup = reinterpret_cast<const ThorQ::Serialization::Group::Message*>(body);

    if (!fbsGroup->Verify(fbsVerifier))
    {
        return;
    }

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

void ThorQ::MessageHandler::onMessageModeration(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsModeration = reinterpret_cast<const ThorQ::Serialization::Moderation::Message*>(body);

    if (!fbsModeration->Verify(fbsVerifier))
    {
        return;
    }
}

void ThorQ::MessageHandler::onMessageSystemID(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsSystemId = reinterpret_cast<const ThorQ::Serialization::SystemId::Message*>(body);

    if (!fbsSystemId->Verify(fbsVerifier))
    {
        return;
    }

    if (fbsSystemId->cmd() != ThorQ::Serialization::SystemId::Command_Submit)
    {
        return;
    }

    std::span<std::uint8_t> systemid(const_cast<std::uint8_t*>(fbsSystemId->data()->data()), fbsSystemId->data()->size());

    if (!ThorQ::SystemID::systemid_validate(systemid))
    {
        return;
    }

    std::string systemID = ThorQ::SystemID::systemid_to_string(systemid);

    fmt::print("SystemID: %s\n", systemID);

    LSql::Connection connection("database.db", LSql::Connection::READWRITE);

    if (!connection.isOpen())
    {
        return;
    }

    LSql::Query query = connection.query("INSERT OR IGNORE INTO system_ids(system_id) VALUES (?1);"
                                                  "SELECT banned_at FROM system_ids WHERE system_id = ?1;");
    query.bind(1, systemID);

    if (!query.step() || query.columnCount() == 0)
    {
        return;
    }

    bool isBanned = (query.column(0).type() == LSql::Type::Null);

    if (isBanned)
    {
        m_server->tryQueueDisconnect(instance->m_peer, false, THORQ_DISCONNECT_REASON::BANNED, m_tokenQueue);
        return;
    }
}

void ThorQ::MessageHandler::onMessageUser(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsUser = reinterpret_cast<const ThorQ::Serialization::User::Message*>(body);

    if (!fbsUser->Verify(fbsVerifier))
    {
        return;
    }
}

void ThorQ::MessageHandler::onMessageVersion(ThorQ::ApiConnection *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsVersion = reinterpret_cast<const ThorQ::Serialization::Version*>(body);

    if (!fbsVersion->Verify(fbsVerifier))
    {
        return;
    }

    fmt::print("[MSG] Version!\n");

    std::uint8_t app = fbsVersion->app();

    const char* name;
    ThorQ::Version currentVersion;
    ThorQ::Version receivedVersion(fbsVersion);

    switch ((THORQ_APP)app) {
    case THORQ_APP::SERVER:
        name = "Server";
        currentVersion = THORQ_VERSION_SERVER;
        break;
    case THORQ_APP::CLIENT:
        name = "Client";
        currentVersion = THORQ_VERSION_CLIENT;
        break;
    case THORQ_APP::LINK:
        name = "Link";
        currentVersion = THORQ_VERSION_LINK;
        break;
    default:
        fmt::print("Client expects invalid version {}[{}]\n", app, receivedVersion.toString());
        return;
    }

    if (receivedVersion == currentVersion)
    {
        fmt::print("Client {}[{}] version matched!\n", name, receivedVersion.toString());
    }
    else
    {
        fmt::print("Client expects {0}[{1}], current is {0}[{2}]\nDisconnecting peer...\n", name, receivedVersion.toString(), currentVersion.toString());
        m_server->tryQueueDisconnect(instance->m_peer, false, THORQ_DISCONNECT_REASON::VERSION_INCOMPATIBLE, m_tokenQueue);
    }
}

void ThorQ::MessageHandler::sendPacket(ThorQ::ApiConnection* instance, std::span<std::uint8_t> data, bool encrypt, std::uint32_t flags, THORQ_CHANNEL channel)
{
    // Get packet
    ENetPacket* packet = ThorQ::Memory::packetGet(ThorQ::calculatePacketSize(data.size(), encrypt));

    // Set flags
    packet->flags = flags;

    if (encrypt)
    {
        // Encode packet
        ThorQ::dataEncode(packet, data, instance->m_crypto);
    }
    else
    {
        // Encode packet
        ThorQ::dataEncode(packet, data);
    }

    // Queue message
    if (!m_server->tryQueueMessage(instance->m_peer, packet, channel, m_tokenQueue)) {
        ThorQ::Memory::packetFree(packet); // If we cant queue it, then free it to avoid a memory leak
    }
}
#endif
