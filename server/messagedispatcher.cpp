#include "messagedispatcher.h"

#include <enet.h>
#include <fmt/core.h>

#include <thorq_message.h>
#include <flatbuffers/flatbuffers.h>
#include <schemas/message_generated.h>
#include <schemas/heartbeat_generated.h>
#include <schemas/version_generated.h>
#include <schemas/crypto_generated.h>
#include <schemas/systemid_generated.h>
#include <schemas/account_generated.h>
#include <schemas/group_generated.h>
#include <schemas/moderation_generated.h>
#include <schemas/announcement_generated.h>
#include <schemas/collar_generated.h>

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

    std::vector<std::uint8_t> message;
/*
    flatbuffers::FlatBufferBuilder builder;
    ThorQ::Serialization::VersionBuilder versionBuilder(builder);
    versionBuilder.

    thorq_payload_version_pack(message, THORQ_APP_LINK, THORQ_VERSION_LINK);
    instance->packetSend(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_version_pack(message, THORQ_APP_CLIENT, THORQ_VERSION_CLIENT);
    instance->packetSend(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_version_pack(message, THORQ_APP_SERVER, THORQ_VERSION_SERVER);
    instance->packetSend(message, THORQ_CHANNEL_MAIN, false, true);

    thorq_payload_heartbeat_pack(message, 500); // TODO: get from config
    instance->packetSend(message, THORQ_CHANNEL_MAIN, false, true);
*/
    char addr[40];
    if (enet_peer_get_ip(event.peer, addr, 40) == 0)
    {
        fmt::print("[{}] Connected\n", addr);
    }
}
void ThorQ::MessageDispatcher::handleEventMessage(const ENetEvent& event)
{
    if (event.peer == nullptr || event.peer->data == nullptr)
    {
        return;
    }

    if (event.channelID > (std::uint8_t)THORQ_CHANNEL::_MAX)
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
    case ThorQ::Serialization::Body_heartbeat:
        handleMessageHeartbeat(instance, fbsMessage->body_as_heartbeat(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(instance, fbsMessage->body_as_version(), fbsVerifier);
        break;/*
    case THORQ_PAYLOAD_ID::CRYPTO:
        handleMessageCrypto(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::SYSTEMID:
        handleMessageSystemID(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::ACCOUNT:
        handleMessageAccount(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::RELATIONSHIP:
        handleMessageRelation(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::SESSION:
        handleMessageSession(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::TOY:
        handleMessageToy(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::COLLAR:
        handleMessageCollar(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::MODERATION:
        handleMessageModeration(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::ACK:
    case THORQ_PAYLOAD_ID::ANNOUNCEMENT:
        fmt::print("Unexpected messageID from client: {}\n", m_buffer[0]);
        break;*/
    case ThorQ::Serialization::Body_MIN:
    case ThorQ::Serialization::Body_MAX:
    default:
        return;
    }
}
void ThorQ::MessageDispatcher::handleEventDisconnect(const ENetEvent& event)
{
    // Get instance
    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    // Remove pointer
    event.peer->data = nullptr;

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

void ThorQ::MessageDispatcher::handleMessageVersion(ThorQ::Instance* instance, const ThorQ::Serialization::Version* fbsVersion, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Version!\n");



    if (fbsVersion->Verify(fbsVerifier))
    {
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
}

void ThorQ::MessageDispatcher::handleMessageHeartbeat(ThorQ::Instance* instance, const ThorQ::Serialization::Heartbeat* fbsHeartbeat, flatbuffers::Verifier fbsVerifier)
{
    if (fbsHeartbeat->Verify(fbsVerifier))
    {
        std::uint32_t interval = m_server->heartbeatInterval();

        if (fbsHeartbeat->interval() != interval)
        {
            // Build flatbuffer
            flatbuffers::FlatBufferBuilder builder;
            auto msg = ThorQ::Serialization::CreateMessage(builder, ThorQ::Serialization::Body_heartbeat, ThorQ::Serialization::CreateHeartbeat(builder, interval).Union());
            builder.Finish(msg);

            // Calculate packet size
            std::size_t size = ThorQ::calculatePacketSize(builder.GetSize(), false);

            //
            ENetPacket* packet = ThorQ::Memory::packetGet(size);
            packet->flags = ENET_PACKET_FLAG_RELIABLE;
            ThorQ::packetEncode(packet, std::span<std::uint8_t>(builder.GetBufferPointer(), builder.GetSize()));
            m_server->tryQueueMessage(instance->m_peer, packet, THORQ_CHANNEL::MAIN, m_tokenQueue);
        }
    }
}

void ThorQ::MessageDispatcher::handleMessageCrypto(ThorQ::Instance* instance, const ThorQ::Serialization::Crypto::Message* fbsCrypto, flatbuffers::Verifier fbsVerifier)
{
    fmt::print("[MSG] Crypto!");
/*
    if (fbsCrypto->Verify(fbsVerifier))
    {
        switch (fbsCrypto->type()) {
        case ThorQ::Serialization::Crypto::MessageType_Request:
        {
            fmt::print("[MSG] Crypto request!");
            instance->m_crypto->generateKeyPair();

            std::vector<std::uint8_t> pubKey;
            pubKey.resize(ThorQ::Crypto::PublicKeyLen);
            instance->m_crypto->getPublicKey(pubKey);
            // Build flatbuffer
            flatbuffers::FlatBufferBuilder builder;
            auto msg = ThorQ::Serialization::CreateMessage(builder, ThorQ::Serialization::Body_crypto,
                                                           ThorQ::Serialization::Crypto::CreateMessageDirect(builder, ThorQ::Serialization::Crypto::MessageType_Establish, &pubKey).Union());
            builder.Finish(msg);

            // Calculate packet size
            std::size_t size = ThorQ::calculatePacketSize(builder.GetSize(), false);

            //

            ENetPacket* packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_RELIABLE);

            ThorQ::packetEncode(packet, builder.GetBufferSpan());

            m_server->tryQueueMessage(instance->m_peer, packet, THORQ_CHANNEL::MAIN, m_tokenQueue);

            ;
            break;
        }
        case ThorQ::Serialization::Crypto::MessageType_Establish:
        {
            fmt::print("[MSG] Crypto establish!");

            std::span<std::uint8_t> data(fbsCrypto->data()->data(), fbsCrypto->data()->size());

            if (instance->m_crypto->agree(data))
            {
                instance->m_verificationData.resize()
            }
            else
            {
                char buf[50];
                enet_peer_get_ip(instance->m_peer, buf, 50);
                fmt::print(stderr, "Failed to create shared secret with %s\n", buf);
                m_server->tryQueueDisconnect(instance->m_peer, false, THORQ_DISCONNECT_REASON::CRYPTO_FAILED, m_tokenQueue);
            }
            break;
        }
        case ThorQ::Serialization::Crypto::MessageType_Verify:
        {
            fmt::print("[MSG] Crypto verify!");
            std::vector<std::uint8_t> data;
            thorq_payload_crypto_verify_unpack(message, data);

            if (instance->cryptoVerify(data))
            {
                fmt::print("[MSG] Crypto verified!");
                thorq_payload_systemid_cmd_pack(response, THORQ_PAYLOAD_SYSTEMID_REQUEST);
                instance->packetSend(response, THORQ_CHANNEL_MAIN, true, true);
                instance->setAuthState(THORQ_STATE_HWID_REQUESTING);
            }
            else
            {
                fmt::print(stderr, "Failed to verify with %s\n", enet_peer_address_str(instance->peer()));
                instance->disconnectPeer(THORQ_DISCONNECT_REASON_CRYPT_FAILED);
            }
            break;
        }
        default:
            fmt::print("[MSG] Crypto \?\?\?!");
            return;
        }
    }
*/
}

void ThorQ::MessageDispatcher::handleMessageSystemID(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
    std::vector<std::uint8_t> response;

    THORQ_PAYLOAD_SYSTEMID cmd;
    thorq_payload_systemid_get_cmd(message, cmd);

    if (cmd != THORQ_PAYLOAD_SYSTEMID_SUBMIT)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SYSTEMID, static_cast<std::uint8_t>(cmd), THORQ_PAYLOAD_ACK_INVALID);
        instance->packetSend(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    std::vector<std::uint8_t> data;
    thorq_payload_systemid_submit_unpack(message, data);


    if (!ThorQ::SystemID::systemid_validate(data))
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SYSTEMID, THORQ_PAYLOAD_SYSTEMID_SUBMIT, THORQ_PAYLOAD_ACK_DENIED);
        instance->packetSend(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    std::string systemID = ThorQ::SystemID::systemid_to_string(data);

    fmt::print("SystemID: %s\n", systemID);

    LSql::Connection connection("database.db", LSql::Connection::READWRITE);

    if (!connection.isOpen())
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SYSTEMID, THORQ_PAYLOAD_SYSTEMID_SUBMIT, THORQ_PAYLOAD_ACK_ERROR);
        instance->packetSend(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    LSql::Query query = connection.query("INSERT OR IGNORE INTO system_ids(system_id) VALUES (?1);"
                                                  "SELECT banned_at FROM system_ids WHERE system_id = ?1;");
    query.bind(1, systemID);

    if (!query.step() || query.columnCount() == 0)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SYSTEMID, THORQ_PAYLOAD_SYSTEMID_SUBMIT, THORQ_PAYLOAD_ACK_ERROR);
        instance->packetSend(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    bool isBanned = (query.column(0).type() == LSql::Type::Null);

    if (isBanned)
    {
        instance->disconnectPeer(THORQ_DISCONNECT_REASON_AUTH_SYSTEMID_BANNED);
        return;
    }
*/}

void ThorQ::MessageDispatcher::handleMessageAccount(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
    std::vector<std::uint8_t> response;

    switch (thorq_payload_account_get_cmd(message)) {
    case THORQ_PAYLOAD_ACCOUNT_REGISTER:
        std::string username, password;
        thorq_payload_account_register_unpack(message, username, password);
        break;
    case THORQ_PAYLOAD_ACCOUNT_DELETE:
        std::string username, password;
        thorq_payload_account_login_unpack(message, username, password);
        break;
    case THORQ_PAYLOAD_ACCOUNT_LOGIN:
        std::string username, password;
        thorq_payload_account_login_unpack(message, username, password);
        break;
    case THORQ_PAYLOAD_ACCOUNT_LOGIN_AUTHTOKEN:
    case THORQ_PAYLOAD_ACCOUNT_LOGOUT:
    default:
        break;
    }

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
    }
*/}

void ThorQ::MessageDispatcher::handleMessageRelation(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
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
*/}

void ThorQ::MessageDispatcher::handleMessageSession(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
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
*/}

void ThorQ::MessageDispatcher::handleMessageModeration(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
*/}

void ThorQ::MessageDispatcher::handleMessageAnnouncement(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
*/}

void ThorQ::MessageDispatcher::handleMessageToy(ThorQ::Instance *instance, const std::vector<uint8_t> &message)
{/*

*/}

void ThorQ::MessageDispatcher::handleMessageCollar(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
    if (instance->sessionState() == THORQ_STATE_SESSION_ACTIVE)
        if (instance->partner() != nullptr)
            instance->partner()->sendMessage(message, true, false);
*/}

void ThorQ::MessageDispatcher::handleMessageAck(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
    (void)instance;
    (void)message;
    fmt::print("Unexpected ack message...\n");
*/}
