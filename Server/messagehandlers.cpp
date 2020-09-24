#include "messagehandlers.h"

#include <QDebug>

#include <utils.h>
#include <enums.h>
#include <crypto.h>
#include <account.h>
#include <version.h>
#include <systemid.h>
#include <thorq_message.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_regkey.h>
#include <thorq_payload_systemid.h>
#include <thorq_payload_account.h>
#include <thorq_payload_event.h>
#include <thorq_payload_announcement.h>
#include <thorq_payload_notification.h>
#include <thorq_payload_collar.h>

#include "utils.h"
#include "session.h"
#include "singletons.h"
#include "authhandler.h"


void handleMessageHeartbeat(ThorQ::Session *instance, const std::vector<uint8_t> &message)
{
    std::uint16_t interval;
    thorq_payload_heartbeat_unpack(message, interval);
    if (interval != 500) // HARDCODED
    {
        std::vector<std::uint8_t> response;
        thorq_payload_heartbeat_pack(response, 500); // HARDCODED
        instance->sendMessage(response, false, true);
    }
}

void handleMessageVersion(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
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
        qDebug().nospace() << QString("Got invalid version %1[%2]").arg(app).arg(version.toString());
		return;
	}


    qDebug().nospace() << QString("Client expects %1[%2], current is %1[%2]")
                          .arg(name).arg(version.toString())
                          .arg(name).arg(currentVersion.toString());

	instance->disconnect(THORQ_DISCONNECT_REASON_VERSION_INCOMPATIBLE);
}

void handleMessageCrypto(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
{
    qDebug() << "message crypto!";
    std::vector<std::uint8_t> response;

    THORQ_PAYLOAD_CRYPTO cmd;
    thorq_payload_crypto_get_cmd(message, cmd);

    switch (cmd) {
    case THORQ_PAYLOAD_CRYPTO_REQUEST:
	{
        qDebug() <<  "Got crypto request!";
		instance->cryptoInit();
        break;
    }
    case THORQ_PAYLOAD_CRYPTO_ESTABLISH:
	{
        qDebug() <<  "Got crypto establish!";
        std::vector<std::uint8_t> data;
        thorq_payload_crypto_get_data(message, data);

		if (!instance->cryptoEstablish(data))
        {
            qWarning() <<  "Failed to create shared secret with" << enet_peer_address_str(instance->peer());
            instance->disconnect(THORQ_DISCONNECT_REASON_CRYPT_FAILED);
        }
        break;
    }
    case THORQ_PAYLOAD_CRYPTO_VERIFY:
	{
        qDebug() <<  "Got crypto verify!";
        std::vector<std::uint8_t> data;
        thorq_payload_crypto_get_data(message, data);

        if (instance->cryptoVerify(data))
		{
            qDebug() <<  "Verified!";
            thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_SYSTEMID_REQ);
            instance->sendMessage(response, true, true);
            instance->setAuthState(THORQ_STATE_AUTH_HWID_REQUESTING);
        }
        else
        {
            qWarning() <<  "Failed to verify with" << enet_peer_address_str(instance->peer());
            instance->disconnect(THORQ_DISCONNECT_REASON_CRYPT_FAILED);
        }
        break;
    }
	default:
        qWarning() << "Crypt???";
		return;
    }
}

void handleMessageSystemID(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
{
    std::vector<std::uint8_t> response;

    THORQ_PAYLOAD_AUTH cmd;
    thorq_payload_auth_get_cmd(message, cmd);

    switch (cmd) {
    case THORQ_PAYLOAD_AUTH_SYSTEMID:
	{
        QByteArray data;
        thorq_payload_auth_get_data(message, data);

        instance->setHwid(data);

        qDebug() << "SystemID:" << ThorQ::systemid_to_string(data);

        switch (ThorQ::AuthHandler::checkSystemID(instance->hwid())) {
        case ThorQ::AuthHandler::REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_OK);
                instance->sendMessage(response, true, true);
                instance->setAuthState(THORQ_STATE_AUTH_OK);
                break;
            }
        case ThorQ::AuthHandler::NOT_REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_REGKEY_REQ);
                instance->sendMessage(response, true, true);
                instance->setAuthState(THORQ_STATE_AUTH_REGKEY_REQUESTING);
                break;
            }
        case ThorQ::AuthHandler::INVALID_SYSTEMID:
            {
                instance->setAuthState(THORQ_STATE_AUTH_NONE);
                instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_SYSTEMID_BANNED);
                break;
            }
        default:
            break;
        }
        break;
    }
    case THORQ_PAYLOAD_AUTH_REGKEY_AWAITING_INPUT:
	{
        instance->setAuthState(THORQ_STATE_AUTH_REGKEY_AWAITING_INPUT);
        break;
    }
    case THORQ_PAYLOAD_AUTH_REGKEY:
	{
        QByteArray data;
        thorq_payload_auth_get_data(message, data);

        switch (ThorQ::AuthHandler::tryRegisterSystemID(instance->hwid(), data)) {
        case ThorQ::AuthHandler::REGISTERED:
        case ThorQ::AuthHandler::RE_REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_OK);
                instance->sendMessage(response, true, true);
                instance->setAuthState(THORQ_STATE_AUTH_OK);
                break;
            }
        case ThorQ::AuthHandler::INVALID_REGKEY:
            {
                thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_REGKEY_REQ);
                instance->sendMessage(response, true, true);
                instance->setAuthState(THORQ_STATE_AUTH_REGKEY_REQUESTING);
                break;
            }
        case ThorQ::AuthHandler::TIMEOUT:
            {
            instance->setAuthState(THORQ_STATE_AUTH_NONE);
            instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_TIMEOUT);
                break;
            }
        case ThorQ::AuthHandler::INVALID_SYSTEMID:
            {
                instance->setAuthState(THORQ_STATE_AUTH_NONE);
                instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_SYSTEMID_BANNED);
                break;
            }
        default:
            break;
        }
        break;
    }
	default:
        qDebug() << "Unexpected message:" << cmd;
		break;
	}
}

void handleMessageRegKey(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
{
    std::vector<std::uint8_t> response;

    THORQ_PAYLOAD_AUTH cmd;
    thorq_payload_auth_get_cmd(message, cmd);

    switch (cmd) {
    case THORQ_PAYLOAD_AUTH_SYSTEMID:
    {
        QByteArray data;
        thorq_payload_auth_get_data(message, data);

        instance->setHwid(data);

        qDebug() << "SystemID:" << ThorQ::systemid_to_string(data);

        switch (ThorQ::AuthHandler::checkSystemID(instance->hwid())) {
        case ThorQ::AuthHandler::REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_OK);
                instance->sendMessage(response, true, true);
                instance->setAuthState(THORQ_STATE_AUTH_OK);
                break;
            }
        case ThorQ::AuthHandler::NOT_REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_REGKEY_REQ);
                instance->sendMessage(response, true, true);
                instance->setAuthState(THORQ_STATE_AUTH_REGKEY_REQUESTING);
                break;
            }
        case ThorQ::AuthHandler::INVALID_SYSTEMID:
            {
                instance->setAuthState(THORQ_STATE_AUTH_NONE);
                instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_SYSTEMID_BANNED);
                break;
            }
        default:
            break;
        }
        break;
    }
    case THORQ_PAYLOAD_AUTH_REGKEY_AWAITING_INPUT:
    {
        instance->setAuthState(THORQ_STATE_AUTH_REGKEY_AWAITING_INPUT);
        break;
    }
    case THORQ_PAYLOAD_AUTH_REGKEY:
    {
        QByteArray data;
        thorq_payload_auth_get_data(message, data);

        switch (ThorQ::AuthHandler::tryRegisterSystemID(instance->hwid(), data)) {
        case ThorQ::AuthHandler::REGISTERED:
        case ThorQ::AuthHandler::RE_REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_OK);
                instance->sendMessage(response, true, true);
                instance->setAuthState(THORQ_STATE_AUTH_OK);
                break;
            }
        case ThorQ::AuthHandler::INVALID_REGKEY:
            {
                thorq_payload_auth_pack(response, THORQ_PAYLOAD_AUTH_REGKEY_REQ);
                instance->sendMessage(response, true, true);
                instance->setAuthState(THORQ_STATE_AUTH_REGKEY_REQUESTING);
                break;
            }
        case ThorQ::AuthHandler::TIMEOUT:
            {
            instance->setAuthState(THORQ_STATE_AUTH_NONE);
            instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_TIMEOUT);
                break;
            }
        case ThorQ::AuthHandler::INVALID_SYSTEMID:
            {
                instance->setAuthState(THORQ_STATE_AUTH_NONE);
                instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_SYSTEMID_BANNED);
                break;
            }
        default:
            break;
        }
        break;
    }
    default:
        qDebug() << "Unexpected message:" << cmd;
        break;
    }
}

void handleMessageAccount(ThorQ::Session *instance, const std::vector<uint8_t> &message)
{
    std::vector<std::uint8_t> response;

    QString username, password;
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

            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_LOGIN, 0, THORQ_PAYLOAD_ACK_OK, username);
            instance->sendMessage(response, true, true);
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_LOGIN, 0, THORQ_PAYLOAD_ACK_DENIED, "Username/Password incorrect!");

            instance->sendMessage(response, true, true);
        }
    }
    else
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_LOGIN, 0, THORQ_PAYLOAD_ACK_NO_CHANGE, username);
        instance->sendMessage(response, true, true);
    }
}

void handleMessageSession(ThorQ::Session *instance)
{
    std::vector<std::uint8_t> response;

    if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
    {
        instance->setLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);

        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_LOGOUT, 0, THORQ_PAYLOAD_ACK_OK);
        instance->sendMessage(response, true, true);
    }
    else
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_LOGOUT, 0, THORQ_PAYLOAD_ACK_NO_CHANGE);
        instance->sendMessage(response, true, true);
    }
}

void handleMessageFriend(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
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

            QList<ThorQ::Session*> instances = g_sessions.toList();

            for (ThorQ::Session* i : instances)
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
            QString username;
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

            QSet<ThorQ::Session*> targetInstances = (*it)->instances();

            if (targetInstances.isEmpty())
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_PAYLOAD_ACK_DENIED, username + "is not online");
                instance->sendMessage(response, true);
                return;
            }

            for (ThorQ::Session* otherInstance : (*it)->instances())
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
            QString name;
            thorq_payload_command_get_data(message, name);

            ThorQ::Session* otherInstance = g_sessions->get(name);

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
	case THORQ_COMMAND_ID_SESSION_DENY:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            QString name;
            thorq_payload_command_get_data(message, name);

            ThorQ::Session* otherInstance = g_sessions->get(name);

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

void handleMessageRoom(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
{
}

void handleMessageModeration(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
{
}

void handleMessageAnnouncement(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
{
}

void handleMessageCollar(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
{
    if (instance->sessionState() == THORQ_STATE_SESSION_ACTIVE)
        if (instance->partner() != nullptr)
            instance->partner()->sendMessage(message, true, false);
}

void handleMessageAck(ThorQ::Session* instance, const std::vector<std::uint8_t>& message)
{
    (void)instance;
    (void)message;
    thorq_debug("Unexpected ack message...")
}
