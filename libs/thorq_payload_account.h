#ifndef THORQ_PAYLOAD_ACCOUNT_H
#define THORQ_PAYLOAD_ACCOUNT_H

#include <QString>
#include <vector>

#include "enums.h"
#include "constants.h"

/// @enum THORQ_PAYLOAD_ACCOUNT_CMD
enum THORQ_PAYLOAD_ACCOUNT : std::uint8_t
{
    THORQ_PAYLOAD_ACCOUNT_CMD_RESERVE_DISCORDID, ///< Try to reserve a discordID
    THORQ_PAYLOAD_ACCOUNT_CMD_RESERVE_USERNAME,  ///< Try to reserve a username
    THORQ_PAYLOAD_ACCOUNT_CMD_REGISTER,          ///< Register an account
    THORQ_PAYLOAD_ACCOUNT_CMD_DELETE,            ///< Delete an account (requires password)
    THORQ_PAYLOAD_ACCOUNT_CMD_LOGIN,             ///< Log in normally
    THORQ_PAYLOAD_ACCOUNT_CMD_LOGIN_AUTHTOKEN,   ///< Log in with authtoken
    THORQ_PAYLOAD_ACCOUNT_CMD_LOGOUT             ///< Log out, removes any authtoken connected to this hwid
};

inline bool thorq_payload_account_is_valid(const std::vector<std::uint8_t>& payload)
{
    if (payload[0] != THORQ_PAYLOAD_ID_ACCOUNT) return false;

    if (payload.size() >= 2)
    {
		switch (payload[1]) {
		case THORQ_PAYLOAD_ACCOUNT_CMD_RESERVE_DISCORDID:
			return payload.size() >= THORQ_DISCORDID_LEN_MIN + 2 && payload.size() <= THORQ_DISCORDID_LEN_MAX + 2;
		case THORQ_PAYLOAD_ACCOUNT_CMD_RESERVE_USERNAME:
			return payload.size() >= THORQ_USERNAME_LEN_MIN + 2 && payload.size() <= THORQ_USERNAME_LEN_MAX + 2;
		case THORQ_PAYLOAD_ACCOUNT_CMD_REGISTER:
			return payload.size() > 5
					&& payload[2] >= THORQ_USERNAME_LEN_MIN
					&& payload[2] <= THORQ_USERNAME_LEN_MAX
					&& payload[3] >= THORQ_PASSWORD_LEN_MIN
					&& payload[3] <= THORQ_PASSWORD_LEN_MAX
					&& payload[4] >= THORQ_DISCORDID_LEN_MIN
					&& payload[4] <= THORQ_DISCORDID_LEN_MAX
					&& payload.size() == 5 + payload[2] + payload[3] + payload[4];
		case THORQ_PAYLOAD_ACCOUNT_CMD_DELETE:
			return payload.size() > 3
					&& payload[1] >= THORQ_PASSWORD_LEN_MIN
					&& payload[1] <= THORQ_PASSWORD_LEN_MAX
					&& payload.size() == 3 + payload[2];
		case THORQ_PAYLOAD_ACCOUNT_CMD_LOGIN:
			return payload.size() > 4
					&& payload[2] >= THORQ_USERNAME_LEN_MIN
					&& payload[2] <= THORQ_USERNAME_LEN_MAX
					&& payload[3] >= THORQ_PASSWORD_LEN_MIN
					&& payload[3] <= THORQ_PASSWORD_LEN_MAX
					&& payload.size() == 4 + payload[2] + payload[3];
		case THORQ_PAYLOAD_ACCOUNT_CMD_LOGIN_AUTHTOKEN:
			return payload.size() == THORQ_AUTHTOKEN_LEN + 2;
		case THORQ_PAYLOAD_ACCOUNT_CMD_LOGOUT:
			return payload.size() == 3;
		default:
			return false;
		}
    }

	return false;
}

inline void thorq_payload_account_get_cmd(const std::vector<std::uint8_t>& payload, THORQ_PAYLOAD_ACCOUNT& id)
{
    id = static_cast<THORQ_PAYLOAD_ACCOUNT>(payload[1]);
}

inline void thorq_payload_account_reserve_discordid_pack(std::vector<std::uint8_t>& payload, const QString& discordID)
{
	QStringRef discordIdRef(&discordID);
	discordIdRef.truncate(THORQ_DISCORDID_LEN_MAX);
	QByteArray discordIdBytes = discordIdRef.toUtf8();

	payload.resize(2 + discordIdBytes.size());
	payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
	payload[1] = THORQ_PAYLOAD_ACCOUNT_CMD_RESERVE_DISCORDID;

	memcpy(payload.data() + 2, discordIdBytes.data(), discordIdBytes.size());
}
inline void thorq_payload_account_reserve_discordid_unpack(const std::vector<std::uint8_t>& payload, QString& discordID)
{
    discordID = QString::fromUtf8((const char*)payload.data() + 2, payload.size() - 2);
}

inline void thorq_payload_account_reserve_username_pack(std::vector<std::uint8_t>& payload, const QString& username)
{
	QStringRef usernameRef(&username);
	usernameRef.truncate(THORQ_USERNAME_LEN_MAX);
	QByteArray usernameBytes = usernameRef.toUtf8();

	payload.resize(2 + usernameBytes.size());
	payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
	payload[1] = THORQ_PAYLOAD_ACCOUNT_CMD_RESERVE_USERNAME;

	memcpy(payload.data() + 2, usernameBytes.data(), usernameBytes.size());
}
inline void thorq_payload_account_reserve_username_unpack(const std::vector<std::uint8_t>& payload, QString& username)
{
    username = QString::fromUtf8((const char*)payload.data() + 2, payload.size() - 2);
}

inline void thorq_payload_account_register_pack(std::vector<std::uint8_t>& payload, const QString& username, const QString& password, const QString& discordID)
{
	QStringRef usernameRef(&username);
	usernameRef.truncate(THORQ_USERNAME_LEN_MAX);
	QByteArray usernameBytes = usernameRef.toUtf8();

	QStringRef passwordRef(&password);
	passwordRef.truncate(THORQ_PASSWORD_LEN_MAX);
	QByteArray passwordBytes = passwordRef.toUtf8();

	QStringRef discordIdRef(&discordID);
	discordIdRef.truncate(THORQ_DISCORDID_LEN_MAX);
	QByteArray discordIdBytes = discordIdRef.toUtf8();

	payload.resize(5 + usernameBytes.size() + passwordBytes.size());
	payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
	payload[1] = THORQ_PAYLOAD_ACCOUNT_CMD_LOGIN;
	payload[2] = usernameBytes.size();
	payload[3] = passwordBytes.size();
	payload[4] = discordIdBytes.size();

	memcpy(payload.data() + 5, usernameBytes.data(), usernameBytes.size());
	memcpy(payload.data() + 5 + usernameBytes.size(), passwordBytes.data(), passwordBytes.size());
	memcpy(payload.data() + 5 + usernameBytes.size() + passwordBytes.size(), discordIdBytes.data(), discordIdBytes.size());
}
inline void thorq_payload_account_register_unpack(const std::vector<std::uint8_t>& payload, QString& username, QString& password, QString& discordID)
{
    const char* data = (const char*)payload.data() + 5;

    username  = QString::fromUtf8(data, payload[2]);
    data += payload[2];

    password  = QString::fromUtf8(data, payload[3]);
    data += payload[3];

    discordID = QString::fromUtf8(data, payload[4]);
}

inline void thorq_payload_account_delete_pack(std::vector<std::uint8_t>& payload, const QString& password)
{
	QStringRef passwordRef(&password);
	passwordRef.truncate(THORQ_PASSWORD_LEN_MAX);
	QByteArray passwordBytes = passwordRef.toUtf8();

    payload.resize(2 +  passwordBytes.size());
	payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
    payload[1] = THORQ_PAYLOAD_ACCOUNT_CMD_LOGIN;

    memcpy(payload.data() + 2, passwordBytes.data(), passwordBytes.size());
}
inline void thorq_payload_account_delete_unpack(const std::vector<std::uint8_t>& payload, QString& password)
{
    password = QString::fromUtf8((const char*)payload.data() + 2, payload.size() - 2);
}

inline void thorq_payload_account_login_pack(std::vector<std::uint8_t>& payload, const QString& username, const QString& password)
{
	QStringRef usernameRef(&username);
	usernameRef.truncate(THORQ_USERNAME_LEN_MAX);
	QByteArray usernameBytes = usernameRef.toUtf8();

	QStringRef passwordRef(&password);
	passwordRef.truncate(THORQ_PASSWORD_LEN_MAX);
	QByteArray passwordBytes = passwordRef.toUtf8();

	payload.resize(4 + usernameBytes.size() + passwordBytes.size());
    payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
	payload[1] = THORQ_PAYLOAD_ACCOUNT_CMD_LOGIN;
	payload[2] = usernameBytes.size();
    payload[3] = passwordBytes.size();

	memcpy(payload.data() + 4, usernameBytes.data(), usernameBytes.size());
	memcpy(payload.data() + 4 + usernameBytes.size(), passwordBytes.data(), passwordBytes.size());
}
inline void thorq_payload_account_login_unpack(const std::vector<std::uint8_t>& payload, QString& username, QString& password)
{
    const char* data = (const char*)payload.data() + 4;

    username  = QString::fromUtf8(data, payload[2]);
    data += payload[2];

    password  = QString::fromUtf8(data, payload[3]);
}

inline void thorq_payload_account_login_authtoken_pack(std::vector<std::uint8_t>& payload, const QByteArray& authtoken)
{
	payload.resize(2 + authtoken.size());
	payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
	payload[1] = THORQ_PAYLOAD_ACCOUNT_CMD_LOGIN_AUTHTOKEN;

    memcpy(payload.data() + 2, authtoken.data(), authtoken.size());
}
inline void thorq_payload_account_login_authtoken_unpack(const std::vector<std::uint8_t>& payload, QByteArray& authtoken)
{
    authtoken.resize(payload.size() - 2);
    memcpy(authtoken.data(), payload.data(), payload.size() - 2);
}

inline void thorq_payload_account_logout_pack(std::vector<std::uint8_t>& payload, std::uint8_t data)
{
    payload.resize(3);
    payload[0] = THORQ_PAYLOAD_ID_ACCOUNT;
	payload[1] = THORQ_PAYLOAD_ACCOUNT_CMD_LOGOUT;
    payload[2] = data;
}
inline void thorq_payload_account_logout_unpack(const std::vector<std::uint8_t>& payload, std::uint8_t& data)
{
    data = payload[2];
}

#endif // THORQ_PAYLOAD_ACCOUNT_H
