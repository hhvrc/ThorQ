#include "messagedispatcher.h"

#include <enet.h>
#include <fmt/core.h>

#include <thorq_message.h>
#include <flatbuffers/flatbuffers.h>
#include <schemas/heartbeat_generated.h>
#include <schemas/version_generated.h>
#include <schemas/crypto_generated.h>
#include <schemas/systemid_generated.h>
#include <schemas/account_generated.h>
#include <schemas/session_generated.h>
#include <schemas/relationship_generated.h>
#include <schemas/moderation_generated.h>
#include <schemas/announcement_generated.h>
#include <schemas/collar_generated.h>

#include "utils.h"
#include "server.h"
#include "account.h"
#include "instance.h"

ThorQ::MessageDispatcher::MessageDispatcher(ThorQ::Server* server)
    : m_server(server)
    , m_buffer(THORQ_PAYLOAD_LEN_MAX)
    , m_tokenGet(server->m_rxQueue)
    , m_tokenQueue(server->m_txQueue)
    , m_tokenBroadcast(server->m_broadcastQueue)
    , m_tokenDisconnect(server->m_disconnectQueue)
{
}

void ThorQ::MessageDispatcher::DispatchEvent(const ENetEvent& event)
{
    if (event.channelID > (std::uint8_t)THORQ_CHANNEL::_MAX)
    {
        return;
    }

    ThorQ::Instance* instance = reinterpret_cast<ThorQ::Instance*>(event.peer->data);

    {
        std::unique_lock l(instance->l_crypto);
        if (!ThorQ::packetDecode(event.packet, m_buffer, instance->m_crypto))
        {
            return;
        }
    }

    if (m_buffer[0] > (std::uint8_t)THORQ_PAYLOAD_ID::_MAX)
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

    switch ((THORQ_PAYLOAD_ID)m_buffer[0]) {
    case THORQ_PAYLOAD_ID::HEARTBEAT:
        handleMessageHeartbeat(instance, m_buffer);
        break;
    case THORQ_PAYLOAD_ID::VERSION:
        handleMessageVersion(instance, m_buffer);
        break;
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
        break;
    case THORQ_PAYLOAD_ID::_MAX:
    case THORQ_PAYLOAD_ID::_INVALID:
        return;
    }
}

void ThorQ::MessageDispatcher::handleMessageHeartbeat(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
    fmt::print("[MSG] Heartbeat!");

    flatbuffers::Verifier verifier(message.data(), message.size());

    const ThorQ::Serialization::Heartbeat* heartbeat = flatbuffers::GetRoot<ThorQ::Serialization::Heartbeat>(message.data());

    if (heartbeat->interval() !=  g_heartbeatSetPoint)
    {
        flatbuffers::FlatBufferBuilder builder;
        auto offset = ThorQ::Serialization::CreateHeartbeat(builder, g_heartbeatSetPoint);
        builder.Finish(offset);

        instance->packetSend(instance->packetEncode(builder.GetBufferPointer(), builder.GetSize(), false, false), THORQ_CHANNEL_MAIN);
    }
*/}

void ThorQ::MessageDispatcher::handleMessageVersion(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
    fmt::print("[MSG] Version!");

    flatbuffers::Verifier verifier(message.data(), message.size());

    const ThorQ::Serialization::Version* version = flatbuffers::GetRoot<ThorQ::Serialization::Version>(message.data());

    version->Verify(verifier);

    ThorQ::Version currentVersion;

    const char* name;

    switch (version->app()) {
    case ThorQ::Serialization::App_Server:
        name = "server";
        currentVersion = THORQ_VERSION_SERVER;
        break;
    case ThorQ::Serialization::App_Client:
        name = "client";
        currentVersion = THORQ_VERSION_CLIENT;
        break;
    case ThorQ::Serialization::App_Link:
        name = "link";
        currentVersion = THORQ_VERSION_LINK;
        break;
    default:
        fmt::print("Client expects invalid version %i[%s]\n", version->app(), version.toString());
        return;
    }

    fmt::print("Client expects %s[%s], current is %s[%s]\n", name, version.toString(), name, currentVersion.toString());

    instance->disconnectPeer(THORQ_DISCONNECT_REASON_VERSION_INCOMPATIBLE);
*/}

void ThorQ::MessageDispatcher::handleMessageCrypto(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{/*
    fmt::print("[MSG] Crypto!");

    flatbuffers::Verifier verifier(message.data(), message.size());

    const ThorQ::Serialization::Crypto::Command* crypto = flatbuffers::GetRoot<ThorQ::Serialization::Crypto::Command>(message.data());

    crypto->Verify(verifier);

    switch (crypto->type()) {
    case ThorQ::Serialization::Crypto::Type_Request:
    {
        fmt::print("[MSG] Crypto request!");
        instance->cryptoInit();
        break;
    }
    case ThorQ::Serialization::Crypto::Type_Establish:
    {
        fmt::print("[MSG] Crypto establish!");
        std::vector<std::uint8_t> data;

        if (!instance->cryptoEstablish(crypto->data()))
        {
            fmt::print(stderr, "Failed to create shared secret with %s\n", enet_peer_address_str(instance->peer()));
            instance->disconnectPeer(THORQ_DISCONNECT_REASON_CRYPT_FAILED);
        }
        break;
    }
    case ThorQ::Serialization::Crypto::Type_Verify:
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
*/}

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
