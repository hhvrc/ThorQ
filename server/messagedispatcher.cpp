#include "messagedispatcher.h"

#include <enet.h>
#include <fmt/core.h>
#include <flatbuffers/flatbuffers.h>

#include <lsql/query.h>
#include <lsql/column.h>
#include <lsql/connection.h>
#include <systemid.h>
#include <thorq_message.h>
#include <schemas/account_generated.h>
#include <schemas/announcement_generated.h>
#include <schemas/collar_generated.h>
#include <schemas/crypto_generated.h>
#include <schemas/file_generated.h>
#include <schemas/version_generated.h>
#include <schemas/systemid_generated.h>
#include <schemas/message_generated.h>
#include <schemas/group_generated.h>
#include <schemas/moderation_generated.h>

#include "server.h"
#include "account.h"
#include "instance.h"
#include "memorymanager.h"

ThorQ::MessageDispatcher::MessageDispatcher(ThorQ::Server* server)
    : m_server(server)
    , m_closing(false)
    , m_buffer(THORQ_PAYLOAD_LEN_MAX)
    , m_tokenGet(server->m_rxQueue)
    , m_tokenQueue(server->m_txQueue)
{
    m_thread = std::thread(&ThorQ::MessageDispatcher::run, this);
}

ThorQ::MessageDispatcher::~MessageDispatcher()
{
    m_closing = true;
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void ThorQ::MessageDispatcher::run()
{
    while (!m_closing)
    {
        ENetEvent event;
        while (m_server->tryGetEvent(event, m_tokenGet))
        {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                handleEventConnection(event);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                handleEventMessage(event);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                handleEventDisconnect(event);
                break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                handleEventTimeout(event);
                break;
            case ENET_EVENT_TYPE_NONE:
            default:
                continue;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ThorQ::MessageDispatcher::handleEventConnection(const ENetEvent& event)
{
    // Dont worry, its ok to have a seemingly dangling pointer here (ENet keeps track of the pointer)

    ThorQ::Instance* instance = new ThorQ::Instance(event.peer);

    flatbuffers::FlatBufferBuilder fbsBuilder;
    flatbuffers::Offset<ThorQ::Serialization::Version> fbsVersion;
    flatbuffers::Offset<ThorQ::Serialization::Message> fbsMessage;

    // Link version
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::LINK, THORQ_VERSION_LINK_MAJOR, THORQ_VERSION_LINK_MINOR, THORQ_VERSION_LINK_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    sendPacket(instance, fbsBuilder.GetBufferSpan(), false, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::MAIN);

    // Client version
    fbsBuilder.Clear();
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::CLIENT, THORQ_VERSION_CLIENT_MAJOR, THORQ_VERSION_CLIENT_MINOR, THORQ_VERSION_CLIENT_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    sendPacket(instance, fbsBuilder.GetBufferSpan(), false, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::MAIN);

    // Server version
    fbsBuilder.Clear();
    fbsVersion = ThorQ::Serialization::CreateVersion(fbsBuilder, (std::uint8_t)THORQ_APP::SERVER, THORQ_VERSION_SERVER_MAJOR, THORQ_VERSION_SERVER_MINOR, THORQ_VERSION_SERVER_PATCH);
    fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_version, fbsVersion.Union());
    fbsBuilder.Finish(fbsMessage);
    sendPacket(instance, fbsBuilder.GetBufferSpan(), false, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::MAIN);

    char addr[40];
    if (enet_peer_get_ip(event.peer, addr, 40) == 0)
    {
        fmt::print("[{}] Connected\n", addr);
    }
}
void ThorQ::MessageDispatcher::handleEventMessage(const ENetEvent& event)
{
    if (event.peer == nullptr || event.peer->data == nullptr ||
        event.channelID > (std::uint8_t)THORQ_CHANNEL::_MAX  ||
        !ThorQ::packetIsValidSize(event.packet))
    {
        return;
    }

    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    m_buffer.resize(ThorQ::calculateDataSize(event.packet));
    if (!ThorQ::packetDecode(event.packet, m_buffer, instance->m_crypto))
    {
        return;
    }

    auto fbsMessage = flatbuffers::GetRoot<ThorQ::Serialization::Message>(m_buffer.data());
    auto fbsVerifier = flatbuffers::Verifier(m_buffer.data(), m_buffer.size());

    if (!fbsMessage->Verify(fbsVerifier))
    {
        return;
    }

    switch ((THORQ_CHANNEL)event.channelID) {
    case THORQ_CHANNEL::MAIN:
        break;
    case THORQ_CHANNEL::EVENTS:
        break;
    case THORQ_CHANNEL::STREAM:
        break;
    case THORQ_CHANNEL::AUTHORITY:
        break;
    case THORQ_CHANNEL::_MAX:
    case THORQ_CHANNEL::_INVALID:
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        handleMessageAccount(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_crypto:
        handleMessageCrypto(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_system_id:
        handleMessageSystemID(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_group:
        handleMessageGroup(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_collar:
        handleMessageCollar(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_moderation:
        handleMessageModeration(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_friend_request:
        handleMessageFriendRequest(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_file:
        handleMessageFile(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_user:
        handleMessageUser(instance, fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_announcement:
        fmt::print("Unexpected messageID from client: {}\n", m_buffer[0]);
        break;
    case ThorQ::Serialization::Body_NONE:
    default:
        return;
    }
}
void ThorQ::MessageDispatcher::handleEventDisconnect(const ENetEvent& event)
{
    // Get instance
    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    // Yeet
    delete instance;

    char addr[40];
    if (enet_peer_get_ip(event.peer, addr, 40) == 0)
    {
        fmt::print("[{}] disconnected\n", addr);
    }
}
void ThorQ::MessageDispatcher::handleEventTimeout(const ENetEvent &event)
{
    char addr[40];
    if (enet_peer_get_ip(event.peer, addr, 40) == 0)
    {
        fmt::print("[{}] timed out\n", addr);
    }
}

void ThorQ::MessageDispatcher::handleMessageAccount(ThorQ::Instance *instance, const void* body, flatbuffers::Verifier fbsVerifier)
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

void ThorQ::MessageDispatcher::handleMessageCollar(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsCollar = reinterpret_cast<const ThorQ::Serialization::Collar::Message*>(body);

    if (!fbsCollar->Verify(fbsVerifier))
    {
        return;
    }

    /*
    if (instance->sessionState() == THORQ_STATE_SESSION_ACTIVE)
        if (instance->partner() != nullptr)
            instance->partner()->sendMessage(message, true, false);
    */
}

void ThorQ::MessageDispatcher::handleMessageCrypto(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
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

        std::array<std::uint8_t, ThorQ::Crypto::PublicKeyLen> pubKey;
        instance->m_crypto.getPublicKey(pubKey);

        // Build flatbuffer
        flatbuffers::FlatBufferBuilder fbsBuilder;
        auto fbsEstablish     = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Establish, fbsBuilder.CreateVector(pubKey.data(), pubKey.size())).Union();
        auto fbsMessage       = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsEstablish);
        fbsBuilder.Finish(fbsMessage);

        sendPacket(instance, fbsBuilder.GetBufferSpan(), false, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::MAIN);
        break;
    }
    case ThorQ::Serialization::Crypto::MessageType_Establish:
    {
        fmt::print("[MSG] Crypto establish!\n");

        if (fbsCrypto->data()->size() != ThorQ::Crypto::PublicKeyLen)
        {
            fmt::print("Got key with invalid length!\n");
            return;
        }

        std::span<std::uint8_t, ThorQ::Crypto::PublicKeyLen> data(
                        const_cast<std::uint8_t*>(fbsCrypto->data()->data()),
                        fbsCrypto->data()->size()
                    );

        if (instance->m_crypto.agreeAsServer(data))
        {
            ThorQ::Crypto::RandomizeBytes(instance->m_verificationData);

            // Build flatbuffer
            flatbuffers::FlatBufferBuilder fbsBuilder;
            auto fbsVerify  = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Verify, fbsBuilder.CreateVector(instance->m_verificationData.data(), instance->m_verificationData.size())).Union();
            auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsVerify);
            fbsBuilder.Finish(fbsMessage);

            sendPacket(instance, fbsBuilder.GetBufferSpan(), true, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::MAIN);
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

            sendPacket(instance, fbsBuilder.GetBufferSpan(), true, ENET_PACKET_FLAG_RELIABLE, THORQ_CHANNEL::MAIN);
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

void ThorQ::MessageDispatcher::handleMessageFile(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsFile = reinterpret_cast<const ThorQ::Serialization::File::Message*>(body);

    if (!fbsFile->Verify(fbsVerifier))
    {
        return;
    }
}
#include <flatbuffers/flexbuffers.h>
void ThorQ::MessageDispatcher::handleMessageFriendRequest(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
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

void ThorQ::MessageDispatcher::handleMessageGroup(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
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

void ThorQ::MessageDispatcher::handleMessageModeration(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsModeration = reinterpret_cast<const ThorQ::Serialization::Moderation::Message*>(body);

    if (!fbsModeration->Verify(fbsVerifier))
    {
        return;
    }
}

void ThorQ::MessageDispatcher::handleMessageSystemID(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
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

void ThorQ::MessageDispatcher::handleMessageUser(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsUser = reinterpret_cast<const ThorQ::Serialization::User::Message*>(body);

    if (!fbsUser->Verify(fbsVerifier))
    {
        return;
    }
}

void ThorQ::MessageDispatcher::handleMessageVersion(ThorQ::Instance *instance, const void *body, flatbuffers::Verifier fbsVerifier)
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

void ThorQ::MessageDispatcher::sendPacket(ThorQ::Instance* instance, std::span<std::uint8_t> data, bool encrypt, std::uint32_t flags, THORQ_CHANNEL channel)
{
    // Get packet
    ENetPacket* packet = ThorQ::Memory::packetGet(ThorQ::calculatePacketSize(data.size(), encrypt));

    // Set flags
    packet->flags = flags;

    if (encrypt)
    {
        // Encode packet
        ThorQ::packetEncode(packet, data, instance->m_crypto);
    }
    else
    {
        // Encode packet
        ThorQ::packetEncode(packet, data);
    }

    // Queue message
    m_server->tryQueueMessage(instance->m_peer, packet, channel, m_tokenQueue);
}
