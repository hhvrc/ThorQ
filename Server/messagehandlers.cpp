#include "messagehandlers.h"

#include <enums.h>
#include <crypto.h>
#include <thorq_message.h>
#include <thorq_payload.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_auth.h>
#include <thorq_payload_command.h>
#include <thorq_payload_command_ack.h>
#include <thorq_payload_notification.h>
#include <thorq_payload_collar.h>

#include "instance.h"
#include "singletons.h"
#include "instancemap.h"

inline bool checkCryptography(ThorQ::Instance* instance)
{

}
inline bool checkAuthority(ThorQ::Instance* instance)
{
	if (!checkCryptography(instance))
		return false;

}
inline bool checkLogin(ThorQ::Instance* instance)
{
	if (!checkAuthority(instance))
		return false;

}
inline bool checkSession(ThorQ::Instance* instance)
{
	if (!checkLogin(instance))
		return false;


}

void handleMessageVersion(ThorQ::Instance* instance, const thorq_payload_t& payload)
{
	std::uint8_t app;
	thorq_version_t version;
	thorq_payload_version_unpack(payload, app, version);

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
		printf("Got invalid version %i[%s]\n", app, version.to_string().c_str());
		fflush(stdout);
		return;
	}

	printf("Client expects %s[%s], current is %s[%s]\n", name, version.to_string().c_str(), name, currentVersion.to_string().c_str());
	fflush(stdout);
}

void handleMessageCrypto(ThorQ::Instance* instance, const thorq_payload_t& payload)
{
	switch (thorq_payload_crypto_get_cmd(payload)) {
	case THORQ_CRYPTO_ESTABLISH:
	{
		qDebug() << "Establishing!";
		std::vector<std::uint8_t> data = thorq_payload_crypto_get_data(payload);

		if (instance->getCrypto()->ready())
			instance->getCrypto()->reset();

		if (instance->getCrypto()->agree(data))
		{
			instance->setCryptoState(THORQ_CRYPTO_STATE_ESTABLISHING);

			thorq_payload_t txPayload;
			thorq_payload_crypto_pack(txPayload, THORQ_CRYPTO_ESTABLISH, instance->getCrypto()->publicKey());
			instance->sendPayload(&txPayload, false, true);
		}
		else
		{
			instance->getCrypto()->reset();
			instance->setCryptoState(THORQ_CRYPTO_STATE_NONE);
		}
	}
		break;
	case THORQ_CRYPTO_VERIFY:
	{
		qDebug() << "Verifying!";
		instance->setCryptoState(THORQ_CRYPTO_STATE_VERIFYING);

		instance->sendPayload(payload, false, true);
	}
		break;
	case THORQ_CRYPTO_OK:
	{
		qDebug() << "CyptOk!";
		instance->setCryptoState(THORQ_CRYPTO_STATE_ACTIVE);
	}
		break;
	default:
		qDebug() << "Crypt???";
		return;
	}
}

void handleMessageAuth(ThorQ::Instance* instance, const thorq_payload_t& payload)
{
	thorq_payload_t txPayload;

	switch (thorq_payload_auth_get_cmd(payload)) {
	case THORQ_AUTH_SYSTEMID_REQ:
		thorq_payload_auth_pack(txPayload, THORQ_AUTH_SYSTEMID, ThorQ::systemid_generate());
		instance->sendPayload(&txPayload);
		instance->setAuthState(THORQ_AUTH_STATE_CHECKING);
		break;
	case THORQ_AUTH_REGKEY_REQ:
		emit RequestingRegistrationKey();
		thorq_payload_auth_pack(txPayload, THORQ_AUTH_REGKEY_AWAITING_INPUT);
		instance->sendPayload(&txPayload);
		instance->setAuthState(THORQ_AUTH_STATE_AWAITING_INPUT);
		break;
	case THORQ_AUTH_OK:
		instance->setAuthState(THORQ_AUTH_STATE_OK);
		break;
	default:
		qDebug() << "Unexpected message:" << thorq_payload_auth_get_cmd(payload);
		break;
	}
}

void handleMessageHeartbeat(ThorQ::Instance* instance)
{
	// TODO: create new message, and send that
	sendMessage(instance->peer(), message, false);
	printf("Heartbeat\n");
	fflush(stdout);
}

void handleMessageCommand(ThorQ::Instance* instance, const thorq_payload_t& payload)
{
	/*
	if (!instance->hasName())
	{
		if (meta == USER_Login)
		{
			std::string name = ExtractString(data, size, sizeof(std::uint32_t));

			if (!registeredInstances->TryAdd(name, instance))
			{
				instance->setName(name);
				instance->clearPartner();

				thorq_payload_t txPayload;
				THORQ_PAYLOAD_ID_NOTIFICATION
				BroadcastMessage(NOTIFY_UserOnline, name);
				instance->SendEncrypted(ACKNOWLEDGE_OK, "Logged in");
			}
			else
			{
				instance->SendEncrypted(ACKNOWLEDGE_Denied, "Callname in use");
			}
		}
		else
		{
			instance->SendEncrypted(ACKNOWLEDGE_Denied, "Please log in");
		}
		return;
	}
	*/

	thorq_command_id_t cmd;

	switch (cmd){
	case THORQ_COMMAND_ID_LOGIN:
	{
		instance->SendEncrypted(ACKNOWLEDGE_Denied, "Already logged in");
		break;
	}
	case THORQ_COMMAND_ID_LOGOUT:
	{
		// Remove from registered
		registeredInstances->Remove(instance->name());

		// Disconnect session if one is ongoing
		if (instance->hasPartner())
		{
			instance->SendEncrypted(NOTIFY_SessionEnded, instance->partner()->name());
			instance->partner()->SendEncrypted(NOTIFY_SessionEnded, instance->name());

			BroadcastMessage(NOTIFY_UserAvailable, instance->partner()->name());

			instance->clearPartner();
		}

		// Announce offline
		if (instance->hasName())
		{
			BroadcastMessage(NOTIFY_UserOffline, instance->name());
			instance->setName("");
		}
		instance->SendEncrypted(ACKNOWLEDGE_OK, "Logged out");
		break;
	}
	case THORQ_COMMAND_ID_GET_USER_LIST:
	{
		std::vector<ThorQ::Instance*> instances = registeredInstances->GetInstances();

		for (ThorQ::Instance* i : instances)
		{
			std::uint32_t txFlag = NOTIFY_UserOnline;

			txFlag |= i->hasCollar() ? FLAG_CollarConnected : 0;

			instance->SendEncrypted(txFlag, i->name());
		}
		break;
	}
	case THORQ_COMMAND_ID_SESSION_REQUEST:
	{
		std::string name = ExtractString(data, size, sizeof(std::uint32_t));

		ThorQ::Instance* otherInstance = registeredInstances->GetInstance(name);

		if (otherInstance == nullptr)
		{
			instance->SendEncrypted(ACKNOWLEDGE_Denied, name + " is not online");
			return;
		}

		otherInstance->requestOn(instance);
		break;
	}
	case THORQ_COMMAND_ID_SESSION_ACCEPT
	{
		std::string name = ExtractString(data, size, sizeof(std::uint32_t));

		ThorQ::Instance* otherInstance = registeredInstances->GetInstance(name);

		if (otherInstance == nullptr)
		{
			instance->SendEncrypted(ACKNOWLEDGE_Denied, name + " is not online");
			return;
		}

		instance->requestAcceptFrom(otherInstance);

		BroadcastMessage(NOTIFY_UserInSession, instance->name());
		BroadcastMessage(NOTIFY_UserInSession, otherInstance->name());
		break;
	}
	case THORQ_COMMAND_ID_SESSION_DENY:
	{
		std::string name = ExtractString(data, size, sizeof(std::uint32_t));

		ThorQ::Instance* otherInstance = registeredInstances->GetInstance(name);

		if (otherInstance == nullptr)
		{
			instance->SendEncrypted(ACKNOWLEDGE_Denied, name + " is not online");
			return;
		}

		instance->partner()->setSessionState(THORQ_SESSION_STATE_NONE);
		break;
	}
	case THORQ_COMMAND_ID_SESSION_LEAVE:
		instance->setSessionState(THORQ_SESSION_STATE_NONE);
		break;
	case THORQ_COMMAND_ID_SET_SELF_STATE:
		break;
	}
}

void handleMessageCommandAck(ThorQ::Instance* instance, const thorq_payload_t& payload)
{

}

void handleMessageNotification(ThorQ::Instance* instance, const thorq_payload_t& payload)
{

}

void handleMessageCollar(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message)
{
	if (instance->sessionState() == THORQ_SESSION_STATE_ACTIVE)
		if (instance->partner() != nullptr)
			instance->partner()->sendMessage(message, true, false);
}
