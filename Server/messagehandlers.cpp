#include "messagehandlers.h"

#include <log.h>
#include <enums.h>
#include <crypto.h>
#include <systemid.h>
#include <thorq_message.h>
#include <thorq_payload.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_auth.h>
#include <thorq_payload_event.h>
#include <thorq_payload_command.h>
#include <thorq_payload_command_ack.h>
#include <thorq_payload_announcement.h>
#include <thorq_payload_notification.h>
#include <thorq_payload_collar.h>

#include "utils.h"
#include "instance.h"
#include "singletons.h"
#include "instancemap.h"
#include "authhandler.h"

void handleMessageVersion(ThorQ::Instance* instance, const thorq_payload_t* payload)
{
	std::uint8_t app;
	thorq_version_t version;
    thorq_payload_version_unpack(*payload, app, version);

	thorq_version_t currentVersion;

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
		thorq_debug_fmt("Got invalid version %i[%s]\n", app, version.to_string().c_str());
		fflush(stdout);
		return;
	}

	thorq_debug_fmt("Client expects %s[%s], current is %s[%s]\n", name, version.to_string().c_str(), name, currentVersion.to_string().c_str());
	fflush(stdout);

	instance->disconnect(THORQ_DISCONNECT_REASON_VERSION_INCOMPATIBLE);
}

void handleMessageCrypto(ThorQ::Instance* instance, const thorq_payload_t* payload)
{
    thorq_payload_t response;

    thorq_crypto_cmd_t cmd;
    thorq_payload_crypto_get_cmd(*payload, cmd);

    switch (cmd) {
    case THORQ_CRYPTO_REQUEST:
    {
        thorq_debug("Got request!")
        instance->cryptoInit();

        thorq_debug("Sent public key!")
        break;
    }
	case THORQ_CRYPTO_ESTABLISH:
	{
        thorq_debug("Got public key!")

        std::vector<std::uint8_t> data;
        thorq_payload_crypto_get_data(*payload, data);

        if (instance->cryptoEstablish(data))
        {
            thorq_debug("Created shared secret!")
        }
        else
        {
            thorq_debug("Failed to create shared secret!")
            instance->disconnect(THORQ_DISCONNECT_REASON_CRYPT_FAILED);
        }
        break;
    }
	case THORQ_CRYPTO_VERIFY:
	{
        thorq_debug("Verifying!")

        std::vector<std::uint8_t> data;
        thorq_payload_crypto_get_data(*payload, data);

        if (instance->cryptoVerify(data))
        {
            thorq_debug("Verified with client!")
            thorq_payload_auth_pack(response, THORQ_AUTH_SYSTEMID_REQ);
            instance->sendPayload(&response, true, true);
            instance->setAuthState(THORQ_AUTH_STATE_HWID_REQUESTING);
        }
        else
        {
            thorq_debug("Failed verify with client!")
            instance->disconnect(THORQ_DISCONNECT_REASON_CRYPT_FAILED);
        }
        break;
    }
	default:
        thorq_debug("Crypt???")
		return;
    }
}

void handleMessageAuth(ThorQ::Instance* instance, const thorq_payload_t* payload)
{
    thorq_payload_t response;

    thorq_auth_cmd_t cmd;
    thorq_payload_auth_get_cmd(*payload, cmd);

    switch (cmd) {
    case THORQ_AUTH_SYSTEMID:
    {
        thorq_debug("SystemID!")

        std::vector<std::uint8_t> data;
        thorq_payload_auth_get_data(*payload, data);

		instance->hwid() = data;

		thorq_debug_fmt("SystemID: %s\n", ThorQ::systemid_to_string(data).c_str())

        switch (ThorQ::AuthHandler::checkSystemID(instance->hwid())) {
        case ThorQ::AuthHandler::REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_AUTH_OK);
                instance->sendPayload(&response, true, true);
                instance->setAuthState(THORQ_AUTH_STATE_OK);
                break;
            }
        case ThorQ::AuthHandler::NOT_REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_AUTH_REGKEY_REQ);
                instance->sendPayload(&response, true, true);
                instance->setAuthState(THORQ_AUTH_STATE_REGKEY_REQUESTING);
                break;
            }
        case ThorQ::AuthHandler::INVALID_SYSTEMID:
            {
                instance->setAuthState(THORQ_AUTH_STATE_NONE);
                instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_INVALID_SYSTEMID);
                break;
            }
        default:
            break;
        }
        break;
    }
    case THORQ_AUTH_REGKEY_AWAITING_INPUT:
    {
        thorq_debug("Input?")
        instance->setAuthState(THORQ_AUTH_STATE_REGKEY_AWAITING_INPUT);
        break;
    }
    case THORQ_AUTH_REGKEY:
    {
        thorq_debug("RegKey!")

		std::array<std::uint8_t, THORQ_AUTH_REGKEY_LEN> data;
        thorq_payload_auth_get_data(*payload, data);

        switch (ThorQ::AuthHandler::tryRegisterSystemID(instance->hwid(), data)) {
        case ThorQ::AuthHandler::REGISTERED:
        case ThorQ::AuthHandler::RE_REGISTERED:
            {
                thorq_payload_auth_pack(response, THORQ_AUTH_OK);
                instance->sendPayload(&response, true, true);
                instance->setAuthState(THORQ_AUTH_STATE_OK);
                break;
            }
        case ThorQ::AuthHandler::INVALID_REGKEY:
            {
                thorq_payload_auth_pack(response, THORQ_AUTH_REGKEY_REQ);
                instance->sendPayload(&response, true, true);
                instance->setAuthState(THORQ_AUTH_STATE_REGKEY_REQUESTING);
                break;
            }
        case ThorQ::AuthHandler::TIMEOUT:
            {
            instance->setAuthState(THORQ_AUTH_STATE_NONE);
            instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_TIMEOUT);
                break;
            }
        case ThorQ::AuthHandler::INVALID_SYSTEMID:
            {
                instance->setAuthState(THORQ_AUTH_STATE_NONE);
                instance->disconnect(THORQ_DISCONNECT_REASON_AUTH_INVALID_SYSTEMID);
                break;
            }
        default:
            break;
        }
        break;
    }
	default:
        thorq_debug_fmt("Unexpected message: %i", cmd)
		break;
	}
}

void handleMessageHeartbeat(ThorQ::Instance* instance)
{
    thorq_payload_t payload;
    thorq_payload_heartbeat_pack(payload);
    instance->sendPayload(&payload, false, true);
}

void handleMessageCommand(ThorQ::Instance* instance, const thorq_payload_t* payload)
{
    thorq_payload_t response;

    thorq_command_id_t cmd;
    thorq_payload_command_get_id(*payload, cmd);

    if (instance->authState() != THORQ_AUTH_STATE_OK)
    {
        thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_UNAUTHORIZED);
        instance->sendPayload(&response, false, true);
        return;
    }

	switch (cmd){
	case THORQ_COMMAND_ID_LOGIN:
    {
        std::string name;

        thorq_payload_command_get_data(*payload, name);

        if (instance->loginState() == THORQ_LOGIN_STATE_LOGGEDOUT)
        {
			instance->name() = name;
			if (registeredInstances->tryAdd(instance))
			{
                thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_OK, name);
                instance->sendPayload(&response, true, true);
            }
            else
            {
				instance->name().clear();
                thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_DENIED, "Username taken");
                instance->sendPayload(&response, true, true);
            }
        }
        else
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_NO_CHANGE, name);
            instance->sendPayload(&response, true, true);
        }
		break;
	}
	case THORQ_COMMAND_ID_LOGOUT:
	{
        if (instance->loginState() == THORQ_LOGIN_STATE_LOGGEDIN)
        {
            // Remove from registered
            registeredInstances->remove(instance->name());

            instance->setLoginState(THORQ_LOGIN_STATE_LOGGEDOUT);

            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_OK);
            instance->sendPayload(&response, true, true);
        }
        else
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_NO_CHANGE);
            instance->sendPayload(&response, true, true);
        }
		break;
	}
	case THORQ_COMMAND_ID_GET_USER_LIST:
	{
        if (instance->loginState() == THORQ_LOGIN_STATE_LOGGEDIN)
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_OK);

            std::vector<ThorQ::Instance*> instances = registeredInstances->instances();

            for (ThorQ::Instance* i : instances)
            {
                thorq_payload_notification_pack(response, THORQ_NOTIFICATION_USER_ACTIVITY, i->name(), i->activityState());
                instance->sendPayload(&response, true, true);
            }
        }
        else
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_LOGIN_NEEDED);
            instance->sendPayload(&response, true, true);
        }
		break;
	}
	case THORQ_COMMAND_ID_SESSION_REQUEST:
	{
        if (instance->loginState() == THORQ_LOGIN_STATE_LOGGEDIN)
        {
            std::string name;
            thorq_payload_command_get_data(*payload, name);

            ThorQ::Instance* otherInstance = registeredInstances->get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_DENIED, name + "is not online");
                instance->sendPayload(&response, true);
                return;
            }

            instance->requestOn(otherInstance);
        }
        else
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_LOGIN_NEEDED);
            instance->sendPayload(&response, true, true);
        }
		break;
	}
    case THORQ_COMMAND_ID_SESSION_ACCEPT:
    {
        if (instance->loginState() == THORQ_LOGIN_STATE_LOGGEDIN)
        {
            std::string name;
            thorq_payload_command_get_data(*payload, name);

            ThorQ::Instance* otherInstance = registeredInstances->get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_DENIED, name + "is not online");
                instance->sendPayload(&response, true);
                return;
            }

            instance->requestAcceptFrom(otherInstance);
        }
        else
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_LOGIN_NEEDED);
            instance->sendPayload(&response, true, true);
        }
        break;
	}
	case THORQ_COMMAND_ID_SESSION_DENY:
    {
        if (instance->loginState() == THORQ_LOGIN_STATE_LOGGEDIN)
        {
            std::string name;
            thorq_payload_command_get_data(*payload, name);

            ThorQ::Instance* otherInstance = registeredInstances->get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_DENIED, name + "is not online");
                instance->sendPayload(&response, true);
                return;
            }

            instance->requestDenyFrom(otherInstance);
        }
        else
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_LOGIN_NEEDED);
            instance->sendPayload(&response, true, true);
        }
        break;
	}
	case THORQ_COMMAND_ID_SESSION_LEAVE:
    {
        if (instance->loginState() == THORQ_LOGIN_STATE_LOGGEDIN)
        {
            instance->setSessionState(THORQ_SESSION_STATE_NONE);
        }
        else
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_LOGIN_NEEDED);
            instance->sendPayload(&response, true, true);
        }
		break;
    }
	case THORQ_COMMAND_ID_SET_SELF_STATE:
    {
        if (instance->loginState() == THORQ_LOGIN_STATE_LOGGEDIN)
        {
            std::uint8_t state;
            thorq_payload_command_get_data(*payload, state);
            instance->setActivityState(state);
        }
        else
        {
            thorq_payload_command_ack_pack(response, cmd, THORQ_COMMAND_ACK_RESULT_LOGIN_NEEDED);
            instance->sendPayload(&response, true, true);
        }
		break;
    }
    }

}

void handleMessageCommandAck(ThorQ::Instance* instance, const thorq_payload_t* payload)
{
    (void)instance;
    (void)payload;
    thorq_debug("Unexpected ack message...")
}

void handleMessageCollar(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
	if (instance->sessionState() == THORQ_SESSION_STATE_ACTIVE)
		if (instance->partner() != nullptr)
			instance->partner()->sendMessage(message, true, false);
}
