#include "messagehandlers.h"

#include <utils.h>
#include <enums.h>
#include <crypto.h>
#include <account.h>
#include <version.h>
#include <systemid.h>
#include <instance.h>
#include <thorq_message.h>
#include <thorq_payload_ack.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_systemid.h>
#include <thorq_payload_account.h>
#include <thorq_payload_announcement.h>
#include <thorq_payload_session.h>
#include <thorq_payload_collar.h>

#include "utils.h"
#include "config.h"
#include "session.h"
#include "singletons.h"
#include "sqlite/connection.h"
#include "sqlite/column.h"
#include "sqlite/query.h"


void handleMessageHeartbeat(ThorQ::Instance *instance, const std::vector<std::uint8_t> &message)
{
    std::uint16_t interval;
    thorq_payload_heartbeat_unpack(message, interval);

    std::uint32_t setPoint = g_heartbeatSetPoint.load();

    if (interval != setPoint)
    {
        std::vector<std::uint8_t> response;
        thorq_payload_heartbeat_pack(response, setPoint);
        instance->sendMessage(response, THORQ_CHANNEL_MAIN, false, true);
    }
}

void handleMessageVersion(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
    THORQ_APP app;
    ThorQ::Version version;
    thorq_payload_version_unpack(message, app, version);

    ThorQ::Version currentVersion;

	const char* name;

	switch (app) {
	case THORQ_APP_SERVER:
		name = "server";
		currentVersion = THORQ_VERSION_SERVER;
		break;
	case THORQ_APP_CLIENT:
		name = "client";
		currentVersion = THORQ_VERSION_CLIENT;
		break;
	case THORQ_APP_LINK:
		name = "link";
		currentVersion = THORQ_VERSION_LINK;
		break;
	default:
        printf("Client expects invalid version %i[%s]\n", app, version.toString().c_str());
		return;
	}

    printf("Client expects %s[%s], current is %s[%s]\n", name, version.toString().c_str(), name, currentVersion.toString().c_str());

    instance->disconnectPeer(THORQ_DISCONNECT_REASON_VERSION_INCOMPATIBLE);
}

void handleMessageCrypto(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
    printf("[MSG] Crypto!");
    std::vector<std::uint8_t> response;

    THORQ_PAYLOAD_CRYPTO cmd;
    thorq_payload_crypto_get_cmd(message, cmd);

    switch (cmd) {
    case THORQ_PAYLOAD_CRYPTO_REQUEST:
	{
        printf("[MSG] Crypto request!");
		instance->cryptoInit();
        break;
    }
    case THORQ_PAYLOAD_CRYPTO_ESTABLISH:
	{
        printf("[MSG] Crypto establish!");
        std::vector<std::uint8_t> data;
        thorq_payload_crypto_establish_unpack(message, data);

		if (!instance->cryptoEstablish(data))
        {
            fprintf(stderr, "Failed to create shared secret with %s\n", enet_peer_address_str(instance->peer()).c_str());
            instance->disconnectPeer(THORQ_DISCONNECT_REASON_CRYPT_FAILED);
        }
        break;
    }
    case THORQ_PAYLOAD_CRYPTO_VERIFY:
	{
        printf("[MSG] Crypto verify!");
        std::vector<std::uint8_t> data;
        thorq_payload_crypto_verify_unpack(message, data);

        if (instance->cryptoVerify(data))
		{
            printf("[MSG] Crypto verified!");
            thorq_payload_systemid_cmd_pack(response, THORQ_PAYLOAD_SYSTEMID_REQUEST);
            instance->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
            instance->setAuthState(THORQ_STATE_HWID_REQUESTING);
        }
        else
        {
            fprintf(stderr, "Failed to verify with %s\n", enet_peer_address_str(instance->peer()).c_str());
            instance->disconnectPeer(THORQ_DISCONNECT_REASON_CRYPT_FAILED);
        }
        break;
    }
	default:
        printf("[MSG] Crypto \?\?\?!");
		return;
    }
}

void handleMessageSystemID(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
    std::vector<std::uint8_t> response;

    THORQ_PAYLOAD_SYSTEMID cmd;
    thorq_payload_systemid_get_cmd(message, cmd);

    if (cmd != THORQ_PAYLOAD_SYSTEMID_SUBMIT)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SYSTEMID, static_cast<std::uint8_t>(cmd), THORQ_PAYLOAD_ACK_INVALID);
        instance->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    std::vector<std::uint8_t> data;
    thorq_payload_systemid_submit_unpack(message, data);


    if (!ThorQ::SystemID::systemid_validate(data))
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SYSTEMID, THORQ_PAYLOAD_SYSTEMID_SUBMIT, THORQ_PAYLOAD_ACK_DENIED);
        instance->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    std::string systemID = ThorQ::SystemID::systemid_to_string(data);

    printf("SystemID: %s\n", systemID.c_str());

    ThorQ::SQLite::Connection connection("database.db", ThorQ::SQLite::Connection::READWRITE);

    if (!connection.isOpen())
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SYSTEMID, THORQ_PAYLOAD_SYSTEMID_SUBMIT, THORQ_PAYLOAD_ACK_ERROR);
        instance->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    ThorQ::SQLite::Query query = connection.query("INSERT OR IGNORE INTO system_ids(system_id) VALUES (?1);"
                                                  "SELECT banned_at FROM system_ids WHERE system_id = ?1;");
    query.bind(1, systemID.c_str());

    if (!query.step() || query.columnCount() == 0)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SYSTEMID, THORQ_PAYLOAD_SYSTEMID_SUBMIT, THORQ_PAYLOAD_ACK_ERROR);
        instance->sendMessage(response, THORQ_CHANNEL_MAIN, true, true);
        return;
    }

    bool isBanned = (query.column(0).type() == ThorQ::SQLite::Type::Null);

    if (isBanned)
    {
        instance->disconnectPeer(THORQ_DISCONNECT_REASON_AUTH_SYSTEMID_BANNED);
        return;
    }
}

void handleMessageAccount(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
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
        auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [&](const ThorQ::Account* account) -> bool
        {
            return account->username() == username;
        });

        if (it != g_accounts.end())
        {
            qDebug() << username << "logged in";

            instance->setAccount(*it);
            instance->setLoginState(THORQ_STATE_LOGIN_LOGGEDIN);

            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_OK);
            instance->sendMessage(response, true, true);
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_DENIED);

            instance->sendMessage(response, true, true);
        }
    }
    else
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_NO_CHANGE);
        instance->sendMessage(response, true, true);
    }
}

void handleMessageFriend(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
    std::vector<std::uint8_t> response;

    THORQ_COMMAND_ID cmd;
    thorq_payload_command_get_id(message, cmd);

    if (instance->authState() != THORQ_STATE_AUTH_OK)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_UNAUTHORIZED);
        instance->sendMessage(response, false, true);
        return;
    }

    switch (cmd){
    case THORQ_COMMAND_ID_GET_USER_LIST:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_OK);
            instance->sendMessage(response, true, true);

            std::vector<ThorQ::Instance*> instances = g_sessions.toList();

            for (ThorQ::Instance* i : instances)
            {
                if (i->account() != nullptr)
                {
                    thorq_payload_notification_pack(response, THORQ_NOTIFICATION_USER_ACTIVITY, i->account()->username(), i->activityState());
                    instance->sendMessage(response, true, true);
                }
            }
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->sendMessage(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_REQUEST:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string username;
            thorq_payload_command_get_data(message, username);

            auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [&](const ThorQ::Account* account) -> bool
            {
                return account->username() == username;
            });

            if (*it == nullptr)
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_PAYLOAD_ACK_DENIED, username + "is not an account");
                instance->sendMessage(response, true);
                return;
            }

            std::unordered_setThorQ::Instance*> targetInstances = (*it)->instances();

            if (targetInstances.isEmpty())
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_PAYLOAD_ACK_DENIED, username + "is not online");
                instance->sendMessage(response, true);
                return;
            }

            for (ThorQ::Instance* otherInstance : (*it)->instances())
            instance->requestOn(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->sendMessage(response, true, true);
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
                instance->sendMessage(response, true);
                return;
            }

            instance->requestAcceptFrom(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->sendMessage(response, true, true);
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

            auto sit = std::find_if(g_accounts.begin(), g_accounts.end(), [name](const ThorQ::Account* a) -> bool
            {
                if (a == nullptr) return false;

                return a->username() == name;
            });
            ThorQ::Instance* otherInstance = g_sessions .get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_DENIED, name + "is not online");
                instance->sendMessage(response, true);
                return;
            }

            instance->requestDenyFrom(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->sendMessage(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_LEAVE:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            instance->setSessionState(THORQ_STATE_SESSION_NONE);

            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_OK);
            instance->sendMessage(response, true, true);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->sendMessage(response, true, true);
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
            instance->sendMessage(response, true, true);
        }
        break;
    }
    }

}

void handleMessageSession(ThorQ::Instance *instance)
{
    std::vector<std::uint8_t> response;

    if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
    {
        instance->setLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);

        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGOUT, THORQ_PAYLOAD_ACK_OK);
        instance->sendMessage(response, true, true);
    }
    else
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGOUT, THORQ_PAYLOAD_ACK_NO_CHANGE);
        instance->sendMessage(response, true, true);
    }
}

void handleMessageModeration(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
}

void handleMessageAnnouncement(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
}

void handleMessageCollar(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
    if (instance->sessionState() == THORQ_STATE_SESSION_ACTIVE)
        if (instance->partner() != nullptr)
            instance->partner()->sendMessage(message, true, false);
}

void handleMessageAck(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
    (void)instance;
    (void)message;
    printf("Unexpected ack message...\n");
}
