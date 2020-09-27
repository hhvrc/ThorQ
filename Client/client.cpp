#include "client.h"

#include <thread>
#include <chrono>

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wextra"
	#pragma GCC diagnostic ignored "-Wpedantic"
	#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#endif

#define ENET_IMPLEMENTATION
#include <enet.h>

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic pop
#endif

#include <QTime>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QElapsedTimer>

#include <enums.h>
#include <crypto.h>
#include <systemid.h>
#include <thorq_message.h>
#include <thorq_payload_crypto.h>
#include <thorq_payload_account.h>
#include <thorq_payload_collar.h>
#include <thorq_payload_regkey.h>
#include <thorq_payload_version.h>
#include <thorq_payload_systemid.h>
#include <thorq_payload_heartbeat.h>
#include <thorq_payload_announcement.h>
#include <thorq_payload_systemid.h>

#define SCOPELOCK(l) std::scoped_lock lock(const_cast<std::mutex&>(l))

/**
 * @enum THORQ_CLIENT_ACTION
 * @brief Flags to tell client how to behave and what to do
 */
enum THORQ_CLIENT_ACTION
{
    ACTION_Connected      = 1 << 0,  ///< [Toggle] Try to connect, and stay connected
    ACTION_ReConnect      = 1 << 1,  ///< [Signal] Try to reconnect, set this if hostname/port is changed
    ACTION_Login          = 1 << 2,  ///< [Signal] Request server to log in with a given username
    ACTION_Logout         = 1 << 3,  ///< [Signal] Request server to log out
    ACTION_ListUsers      = 1 << 4,  ///< [Signal] Request server to list online users
    ACTION_SessionRequest = 1 << 5,  ///< [Signal] Request server to start a session with a given user
    ACTION_SessionAccept  = 1 << 6,  ///< [Signal] Tell server to accept an given incoming request
    ACTION_SessionDeny    = 1 << 7,  ///< [Signal] Tell server to deny an given incoming request
    ACTION_SessionLeave   = 1 << 8,  ///< [Signal] Tell server to stop an ongoing session
    ACTION_SendRegKey     = 1 << 9,  ///< [Signal] Send registration key to server
    ACTION_RESERVED_11    = 1 << 10,
    ACTION_RESERVED_12    = 1 << 11,
    ACTION_RESERVED_13    = 1 << 12,
    ACTION_RESERVED_14    = 1 << 13,
    ACTION_RESERVED_15    = 1 << 14,
    ACTION_RESERVED_16    = 1 << 15,
    ACTION_TOGGLEACTIONS  = ACTION_Connected ///< All action flags that are meant to be toggled, and not used as signals, these flags will not be cleared after they are read
};

/**
 * @enum THORQ_COLLAR_FLAG
 * @brief Flags to describe current user input
 */
enum THORQ_COLLAR_FLAG
{
    THORQ_COLLAR_FLAG_SHOCK      = 1 << 0, ///< Activate collar shock
    THORQ_COLLAR_FLAG_VIBRATE    = 1 << 1, ///< Activate collar vibration
    THORQ_COLLAR_FLAG_BEEP       = 1 << 2, ///< Activate collar speaker
    THORQ_COLLAR_FLAG_AUTO       = 1 << 3, ///< Auto mode
    THORQ_COLLAR_FLAG_RESERVED_5 = 1 << 4,
    THORQ_COLLAR_FLAG_RESERVED_6 = 1 << 5,
    THORQ_COLLAR_FLAG_RESERVED_7 = 1 << 6,
    THORQ_COLLAR_FLAG_IMPULSE    = 1 << 7,
};

std::string enetaddr_to_str(const ENetAddress* addr)
{
	char buffer[50];
	if (enet_address_get_host_ip(addr, buffer, 50) < 0)
		return "ERROR";
	return std::string(buffer);
}

std::string ExtractString(const std::uint8_t* data, std::size_t dataSize, std::size_t startOffset = 0, std::size_t endOffset = 0)
{
	return std::string(data + startOffset, data + dataSize - endOffset);
}

Client::Client(ENetHost* host)
	: QObject()
	, m_crypto(new ThorQ::Crypto())
	, m_connectionState(THORQ_STATE_CONNECTION_DISCONNECTED)
	, m_cryptoState(THORQ_STATE_CRYPTO_NONE)
	, m_authState(THORQ_STATE_AUTH_NONE)
	, m_loginState(THORQ_STATE_LOGIN_LOGGEDOUT)
	, m_sessionState(THORQ_STATE_SESSION_NONE)
    , m_rtt(0)
    , l_loginInfo()
	, m_username("")
	, l_partnerName()
	, m_partnerName("")
	, l_requestedPartner()
	, m_requestedPartner("")
	, l_requestingPartner()
	, m_requestingPartner("")
	, l_registrationKey()
	, m_registrationKey("")
	, m_actionFlags(0)
	, m_collarState(0)
	, m_thread(new QThread())
	, m_serviceTimer(new QTimer())
    , m_awaitingHeartbeat(false)
    , m_lastCheck(0)
    , m_heartbeatTimer(new QElapsedTimer())
	, m_host(host)
	, m_peer(nullptr)
	, l_requestedHost()
	, m_requestedHostName("")
	, m_requestedHostPort(0)
	, m_address(new ENetAddress())
{
	QObject::connect(this, &Client::PortChanged, this, &Client::Reconnect);
	QObject::connect(this, &Client::AddressChanged, this, &Client::Reconnect);

	reinterpret_cast<QObject*>(this)->moveToThread(m_thread);

	connect(m_serviceTimer, &QTimer::timeout, this, &Client::Service);
	m_serviceTimer->setSingleShot(false);
    m_serviceTimer->setInterval(5);
	m_serviceTimer->start();

	m_thread->start();
}

Client* Client::NewClient()
{
	ENetHost* host = enet_host_create(nullptr, 1, 2, 0, 0);

	if (host == nullptr)
		return nullptr;

	return new Client(host);
}

Client::~Client()
{
	m_thread->quit();
	m_thread->requestInterruption();
	m_thread->wait();

	if (ConnectionState() != THORQ_STATE_CONNECTION_DISCONNECTED)
	{
		Disconnect();
		do { Service(); }
		while (ConnectionState() != THORQ_STATE_CONNECTION_DISCONNECTED);
	}

	if (m_host != nullptr)
	{
		enet_host_destroy(m_host);
	}

	delete m_address;
    delete m_heartbeatTimer;
	delete m_crypto;
	delete m_thread;
}

QString Client::Version()
{
	return QString("ENet-%1.%2.%3").arg(ENET_VERSION_MAJOR).arg(ENET_VERSION_MINOR).arg(ENET_VERSION_PATCH);
}

quint16 Client::Rtt() const
{
    return m_rtt.load();
}

THORQ_STATE_CONNECTION Client::ConnectionState() const
{
	return m_connectionState.load();
}

THORQ_STATE_CRYPTO Client::CryptoState() const
{
	return m_cryptoState.load();
}

THORQ_STATE_AUTH Client::AuthState() const
{
	return m_authState.load();
}

THORQ_STATE_LOGIN Client::LoginState() const
{
	return m_loginState.load();
}

THORQ_STATE_SESSION Client::SessionState() const
{
	return m_sessionState.load();
}

void Client::Connect(const char* address, quint16 port)
{
    SCOPELOCK(l_requestedHost);
    m_requestedHostName = address;
    m_requestedHostPort = port;

	m_actionFlags.fetch_or(ACTION_Connected);
}

void Client::Reconnect()
{
	m_actionFlags.fetch_or(ACTION_ReConnect);
}

void Client::Disconnect()
{
	m_actionFlags.fetch_and(~ACTION_Connected);
}

void Client::Login(const QString &username)
{
    qDebug() << "Login:" << username;
	if (LoginState() != THORQ_STATE_LOGIN_LOGGEDOUT)
		return;

    SCOPELOCK(l_loginInfo);
	m_username = username;

	m_actionFlags.fetch_or(ACTION_Login);
}

void Client::Logout()
{
	if (LoginState() != THORQ_STATE_LOGIN_LOGGEDIN)
		return;

	m_actionFlags.fetch_or(ACTION_Logout);
}

void Client::RequestSession(const QString &username)
{
	SCOPELOCK(l_requestedPartner);
	m_requestedPartner = username;
	m_actionFlags.fetch_or(ACTION_SessionRequest);
}

void Client::AcceptRequest()
{
	m_actionFlags.fetch_or(ACTION_SessionAccept);
}

void Client::DenyRequest()
{
	m_actionFlags.fetch_or(ACTION_SessionDeny);
}

void Client::LeaveSession()
{
	m_actionFlags.fetch_or(ACTION_SessionLeave);
}

// Pack the collar data like this so we can write everything to one atomic variable
// SIZE   #    8 |     8 |       8 |    8 |     8
// OFFSET #   56 |    48 |      40 |   32 |     0
// MASK   # 0xFF |  0xFF |    0xFF | 0xFF |  0xFF
// NAME   # AUTO | SHOCK | VIBRATE | BEEP | FLAGS

void Client::SetShock(std::uint8_t value)
{
    m_collarState.fetch_and(~((0xFFull << 56) | 0xFFull));
    m_collarState.fetch_or(((std::uint64_t)value << 56) | THORQ_COLLAR_FLAG_SHOCK);
}
void Client::SetVibrate(std::uint8_t value)
{
    m_collarState.fetch_and(~((0xFFull << 48) | 0xFFull));
    m_collarState.fetch_or((std::uint64_t(value) << 48) | THORQ_COLLAR_FLAG_VIBRATE);
}
void Client::SetBeep(std::uint8_t value)
{
    m_collarState.fetch_and(~((0xFFull << 40) | 0xFFull));
    m_collarState.fetch_or((std::uint64_t(value) << 40) | THORQ_COLLAR_FLAG_BEEP);
}
void Client::EnableAuto(std::uint8_t value)
{
    m_collarState.fetch_and(~((0xFFull << 32) | 0xFFull));
    m_collarState.fetch_or((std::uint64_t(value) << 32) | THORQ_COLLAR_FLAG_AUTO);
}
void Client::DisableAuto()
{
	m_collarState.fetch_and(~THORQ_COLLAR_FLAG_AUTO);
}
void Client::SendImpulse()
{
	m_collarState.fetch_or(THORQ_COLLAR_FLAG_IMPULSE);
}

void Client::submitRegistrationKey(const QString& regKey)
{
    SCOPELOCK(l_registrationKey);
	m_registrationKey = regKey;

	m_actionFlags.fetch_or(ACTION_SendRegKey);
}

void Client::Service()
{
	ENetEvent event;
	while (enet_host_service(m_host, &event, 0) > 0)
	{
		switch (event.type)
		{
        case ENET_EVENT_TYPE_CONNECT:
			SetConnectionState(THORQ_STATE_CONNECTION_CONNECTED);
			requestEncryptionHandshake();
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			HandleMessage(event.packet);
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			handleDisconnect(event.data);
			break;
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			handleDisconnect(THORQ_DISCONNECT_REASON_TIMEDOUT);
			break;
		case ENET_EVENT_TYPE_NONE:
			break;
		}
	}

	// Gets the actions, and clears the actions that arent toggleables
	uint actions = m_actionFlags.fetch_and(ACTION_TOGGLEACTIONS);

	// Send stuff
	if (ConnectionState() == THORQ_STATE_CONNECTION_CONNECTED)
    {
        if (LoginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            if ((actions & ACTION_Logout) != 0)
            {
                std::vector<std::uint8_t> payload;

                SCOPELOCK(l_loginInfo);
				thorq_payload_account_logout_pack(payload);
                SendPayload(payload, true, true);

				SetLoginState(THORQ_STATE_LOGIN_LOGGINGOUT);
            }
            else
            {
                std::uint64_t collarState = m_collarState.fetch_and(~0xFF);

                if (SessionState() == THORQ_STATE_SESSION_ACTIVE && ((collarState & THORQ_COLLAR_FLAG_IMPULSE) != 0))
                {
                    std::vector<std::uint8_t> payload;
                    thorq_payload_collar_pack(payload, collarState & 0xFF, (collarState >> 56) & 0xFF, (collarState >> 48) & 0xFF, (collarState >> 40) & 0xFF, (collarState >> 32) & 0xFF);
                    SendPayload(payload, true, false);
                }
                else
                {
                    if ((actions & ACTION_SessionRequest) != 0)
                    {
                        std::vector<std::uint8_t> payload;
                        thorq_payload_command_pack(payload, THORQ_COMMAND_ID_SESSION_REQUEST, m_requestedPartner);
                        SendPayload(payload, true, true);
                    }
                    else if ((actions & ACTION_SessionAccept) != 0)
                    {
                        std::vector<std::uint8_t> payload;
                        thorq_payload_command_pack(payload, THORQ_COMMAND_ID_SESSION_ACCEPT, m_requestingPartner);
                        SendPayload(payload, true, true);
                        m_requestingPartner.clear();
                        SetSessionState(THORQ_STATE_SESSION_JOINING);
                    }
                    else if ((actions & ACTION_SessionDeny) != 0)
                    {
                        std::vector<std::uint8_t> payload;
                        thorq_payload_command_pack(payload, THORQ_COMMAND_ID_SESSION_DENY, m_requestingPartner);
                        SendPayload(payload, true, true);
                        m_requestingPartner.clear();
                        SetSessionState(THORQ_STATE_SESSION_NONE);
                    }
                }
            }
        }
        else if (LoginState() == THORQ_STATE_LOGIN_LOGGEDOUT)
        {
            if ((actions & ACTION_SendRegKey) != 0)
            {
                if (AuthState() == THORQ_STATE_AUTH_REGKEY_AWAITING_INPUT)
                {
                    std::vector<std::uint8_t> payload;

                    SCOPELOCK(l_registrationKey);
                    thorq_payload_auth_pack(payload, THORQ_PAYLOAD_AUTH_REGKEY, m_registrationKey.toUtf8());

                    SendPayload(payload, true, true);
                    SetAuthState(THORQ_STATE_AUTH_REGKEY_CHECKING);
                }
            }
            else if ((actions & ACTION_Login) != 0)
            {
                std::vector<std::uint8_t> payload;

                SCOPELOCK(l_loginInfo);
                thorq_payload_login_pack(payload, m_username, m_password);
                SendPayload(payload, true, true);

                SetLoginState(THORQ_STATE_LOGIN_LOGGINGIN);
            }

        }


        /**
         * Disconnects client gracefully
         * If [STATE] ACTION_Connected is not set, then the clients should not be connected
         * If [FLAG]  ACTION_ReConnect is set, then the client should disconnect, and will naturally reconnect again (i love state machines)
         */
		if ((actions & ACTION_ReConnect) != 0 || (actions & ACTION_Connected) == 0)
        {
			SetConnectionState(THORQ_STATE_CONNECTION_DISCONNECTING);
			enet_peer_disconnect(m_peer, 0);
		}
		else
		{
            std::uint64_t elapsed = m_heartbeatTimer->elapsed();

            // If there has been been more than the set interval of ms since last heartbeat got sent,
            // then update the RTT and resend heartbeat
            if ((elapsed - m_lastCheck) > m_heartbeatInterval)
			{
                if (m_awaitingHeartbeat)
                {
                    // We still havent received a heart,
                    // So update the RTT, and the check time
                    SetRtt(elapsed);
                    m_lastCheck = elapsed;
				}
				else
                {
                    // We did get a heartbeat response, so reset the elapsed time, and the check time
                    m_heartbeatTimer->start();
                    m_lastCheck = 0;
				}

                // Send a heartbeat, and set awaiting to true
                std::vector<std::uint8_t> payload;
                thorq_payload_heartbeat_pack(payload, m_heartbeatInterval);
                SendPayload(payload, false, false);
                m_awaitingHeartbeat = true;
			}
		}
	}
	else if (ConnectionState() == THORQ_STATE_CONNECTION_DISCONNECTED)
	{
		if ((actions & ACTION_Connected) != 0)
        {
            bool addressFound = false;

			{
				SCOPELOCK(l_requestedHost);
				if (enet_address_set_host(m_address, m_requestedHostName.toStdString().c_str()) == 0)
				{
					m_address->port = m_requestedHostPort;
                    addressFound = true;
				}
			}

            if (addressFound)
			{
				m_peer = enet_host_connect(m_host, m_address, 4, 0);
				SetConnectionState(THORQ_STATE_CONNECTION_CONNECTING);
			}
			else
			{
				// TODO: something
			}
		}
	}
}

void Client::SetRtt(std::uint16_t rtt)
{
    if (m_rtt != rtt)
    {
        m_rtt = rtt;
        emit RttChanged(rtt);
	}
}

// Cascading setters
void Client::SetConnectionState(THORQ_STATE_CONNECTION newState)
{
	int oldState = m_connectionState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
            SetCryptoState(THORQ_STATE_CRYPTO_NONE);

		emit ConnectionStateChanged(newState);
	}
}
void Client::SetCryptoState(THORQ_STATE_CRYPTO newState)
{
	int oldState = m_cryptoState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetAuthState(THORQ_STATE_AUTH_NONE);
		else
			SetConnectionState(THORQ_STATE_CONNECTION_CONNECTED);

		emit CryptoStateChanged(newState);
	}
}

void Client::SetAuthState(THORQ_STATE_AUTH newState)
{
	int oldState = m_authState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);
		else
			SetCryptoState(THORQ_STATE_CRYPTO_ACTIVE);

		emit AuthStateChanged(newState);
	}
}
void Client::SetLoginState(THORQ_STATE_LOGIN newState)
{
	int oldState = m_loginState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetSessionState(THORQ_STATE_SESSION_NONE);
		else
			SetAuthState(THORQ_STATE_AUTH_OK);

		emit LoginStateChanged(newState);
	}
}
void Client::SetSessionState(THORQ_STATE_SESSION newState)
{
	int oldState = m_sessionState.exchange(newState);

	if (newState != oldState)
	{
		if (newState > oldState)
			SetLoginState(THORQ_STATE_LOGIN_LOGGEDIN);

		emit SessionStateChanged(newState);
	}
}

void Client::SetUsername(const QString &username)
{
    SCOPELOCK(l_loginInfo);
	m_username = username;
}

void Client::SetPartner(const QString &username)
{
	SCOPELOCK(l_partnerName);
	m_partnerName = username;
}

void Client::HandleMessage(ENetPacket* packet)
{
	if (ConnectionState() != THORQ_STATE_CONNECTION_CONNECTED)
		return;

	std::vector<std::uint8_t> message(packet->data, packet->data + packet->dataLength);

    if (!thorq_message_decode(message, m_crypto))
	{
        return;
	}

	switch (message[0]) {
	case THORQ_PAYLOAD_ID_HEARTBEAT:
		if (thorq_payload_heartbeat_is_valid(message))
		{
			handlePayloadHeartbeat(message);
		}
		return;
    case THORQ_PAYLOAD_ID_VERSION:
        if (thorq_payload_version_is_valid(message))
		{
			handlePayloadVersion(message);
		}
		return;
    case THORQ_PAYLOAD_ID_CRYPTO:
        if (thorq_payload_crypto_is_valid(message))
		{
			handlePayloadCrypto(message);
		}
		return;
	case THORQ_PAYLOAD_ID_SYSTEMID:
		if (thorq_payload_systemid_is_valid(message))
		{
			handlePayloadSystemID(message);
		}
		return;
	case THORQ_PAYLOAD_ID_REGKEY:
		if (thorq_payload_regkey_is_valid(message))
		{
			handlePayloadRegKey(message);
		}
		return;
	case THORQ_PAYLOAD_ID_ACCOUNT:
		if (thorq_payload_account_is_valid(message))
		{
			handlePayloadAccount(message);
		}
		return;
	case THORQ_PAYLOAD_ID_RELATION:
		if (thorq_payload_relation_is_valid(message))
		{
			handlePayloadRelation(message);
		}
		return;
	case THORQ_PAYLOAD_ID_SESSION:
		if (thorq_payload_session_is_valid(message))
		{
			handlePayloadSession(message);
		}
		return;
	case THORQ_PAYLOAD_ID_ROOM:
		if (thorq_payload_room_is_valid(message))
		{
			handlePayloadRoom(message);
		}
		return;
	case THORQ_PAYLOAD_ID_MODERATION:
		if (thorq_payload_moderation_is_valid(message))
		{
			handlePayloadModeration(message);
		}
		return;
	case THORQ_PAYLOAD_ID_ANNOUNCEMENT:
		if (thorq_payload_announcement_is_valid(message))
		{
			handlePayloadAnnouncement(message);
		}
		return;
	case THORQ_PAYLOAD_ID_COLLAR:
		if (thorq_payload_collar_is_valid(message))
		{
			handlePayloadCollar(message);
		}
		return;
	case THORQ_PAYLOAD_ID_ACK:
		if (thorq_payload_ack_is_valid(message))
		{
			handlePayloadAck(message);
		}
		return;
	default:
		if (AuthState() != THORQ_STATE_AUTH_OK)
		{
			return;
		}

		break;
	}
}

void Client::handlePayloadHeartbeat(std::vector<std::uint8_t>& payload)
{
	// Set interval from server
	thorq_payload_heartbeat_unpack(payload, m_heartbeatInterval);

	if (m_awaitingHeartbeat)
	{
		m_awaitingHeartbeat = false;
		SetRtt(m_heartbeatTimer->elapsed());
	}
}
void Client::handlePayloadVersion(std::vector<std::uint8_t>& payload)
{
    THORQ_APP app;
    ThorQ::Version version;
    thorq_payload_version_unpack(payload, app, version);

	switch (app) {
	case THORQ_APP_SERVER:
		if (version > THORQ_VERSION_SERVER)
        { qDebug() << tr("Server has updated from %1 to %2").arg(THORQ_VERSION_SERVER.toString()).arg(version.toString()); }
		else if (version < THORQ_VERSION_SERVER)
        { qDebug() << tr("Server had downdated from %1 to %2").arg(THORQ_VERSION_SERVER.toString()).arg(version.toString()); }
		else
		{ qDebug() << tr("Server version compatible"); }
		break;
	case THORQ_APP_CLIENT:
		if (version > THORQ_VERSION_CLIENT)
        { qDebug() << tr("Client has updated from %1 to %2").arg(THORQ_VERSION_CLIENT.toString()).arg(version.toString()); emit Error(tr("New update available!\nClient v%1").arg(version.toString())); }
		else if (version < THORQ_VERSION_CLIENT)
        { qDebug() << tr("Client has downgraded from %1 to %2").arg(THORQ_VERSION_CLIENT.toString()).arg(version.toString()); emit Error(tr("Hello future-person!\nServer expects: Client v%1\nYou have: Client v%2").arg(version.toString()).arg(THORQ_VERSION_CLIENT.toString())); }
		else
		{ qDebug() << tr("Client version compatible"); }
		break;
	case THORQ_APP_LINK:
		if (version > THORQ_VERSION_LINK)
        { qDebug() << tr("Protocol has updated from %1 to %2").arg(THORQ_VERSION_LINK.toString()).arg(version.toString()); emit Error(tr("Version incompatible!\nPlease upgrade")); }
		else if (version < THORQ_VERSION_LINK)
        { qDebug() << tr("Protocol has downgraded from %1 to %2").arg(THORQ_VERSION_LINK.toString()).arg(version.toString()); emit Error(tr("Version inompatible!\nPlease downgrade")); }
		else
		{ qDebug() << tr("Protocol version compatible"); }
		break;
	default:
        qDebug() << tr("Got ivalid version %1[%2]").arg(app).arg(version.toString());
		return;
	}
}
void Client::handlePayloadCrypto(std::vector<std::uint8_t>& payload)
{
    std::vector<std::uint8_t> response;

    THORQ_PAYLOAD_CRYPTO cmd;
    thorq_payload_crypto_get_cmd(payload, cmd);

    switch (cmd) {
	case THORQ_PAYLOAD_CRYPTO_ESTABLISH:
    {
        SetCryptoState(THORQ_STATE_CRYPTO_ESTABLISHING);

        std::vector<std::uint8_t> data;
        thorq_payload_crypto_establish_unpack(payload, data);

        if (m_crypto->ready()) m_crypto->reset();

        if (m_crypto->agree(data))
        {
            thorq_payload_crypto_establish_pack(response, m_crypto->publicKey());
            SendPayload(response, false, true);
		}
		else
		{
			m_crypto->reset();
			SetCryptoState(THORQ_STATE_CRYPTO_NONE);
		}
        break;
    }
    case THORQ_PAYLOAD_CRYPTO_VERIFY:
        SetCryptoState(THORQ_STATE_CRYPTO_VERIFYING);
        SendPayload(payload, true, true);
		break;
    case THORQ_PAYLOAD_CRYPTO_OK:
        SetCryptoState(THORQ_STATE_CRYPTO_ACTIVE);
		break;
	default:
        qDebug() << "CRYPT: Unexpected message:" << cmd;
		return;
	}
}
void Client::handlePayloadSystemID(std::vector<std::uint8_t>& payload)
{
    std::vector<std::uint8_t> response;

	THORQ_PAYLOAD_SYSTEMID cmd;
	thorq_payload_systemid_get_cmd(payload, cmd);

    switch (cmd) {
	case THORQ_PAYLOAD_SYSTEMID_REQ:
		thorq_payload_systemid_data_pack(response, ThorQ::systemid_generate());
        SendPayload(response);
        SetAuthState(THORQ_STATE_AUTH_HWID_CHECKING);
		break;
	case THORQ_PAYLOAD_SYSTEMID_OK:
		SetAuthState(THORQ_STATE_AUTH_OK);
		break;
	default:
        qDebug() << "AUTH: Unexpected message:" << cmd;
		break;
    }
}
void Client::handlePayloadRegKey(std::vector<std::uint8_t>& payload)
{
	std::vector<std::uint8_t> response;

	THORQ_PAYLOAD_REGKEY cmd;
	thorq_payload_regkey_get_cmd(payload, cmd);

	switch (cmd) {
	case THORQ_PAYLOAD_REGKEY_REQ:
		thorq_payload_regkey_pack(response, THORQ_PAYLOAD_REGKEY_AWAITING_INPUT);
		SendPayload(response);
		SetAuthState(THORQ_STATE_AUTH_REGKEY_AWAITING_INPUT);
		emit RequestingRegistrationKey();
		break;
	case THORQ_PAYLOAD_REGKEY_OK:
		SetAuthState(THORQ_STATE_AUTH_OK);
		break;
	default:
		qDebug() << "AUTH: Unexpected message:" << cmd;
		break;
	}
}
void Client::handlePayloadAccount(std::vector<std::uint8_t>& payload)
{
	THORQ_PAYLOAD_ACCOUNT cmd;
	thorq_payload_account_get_cmd(payload, cmd);

	switch (cmd) {
	case THORQ_PAYLOAD_ACCOU
		type_str = "admin";
		break;
	case THORQ_PAYLOAD_ANNOUNCEMENT_TYPE_SYSTEM:
		type_str = "system";
		break;
	}

	emit Announcement("Announcement!");
}
void Client::handlePayloadAnnouncement(std::vector<std::uint8_t>& payload)
{
	QString message;
    THORQ_PAYLOAD_ANNOUNCEMENT_TYPE type;
    THORQ_PAYLOAD_ANNOUNCEMENT_REASON reason;

    thorq_payload_announcement_get_type(payload, type);
    thorq_payload_announcement_get_reason(payload, reason);
    thorq_payload_announcement_get_message(payload, message);

    const char* type_str;
    const char* reason_str;

    switch (type) {
    case THORQ_PAYLOAD_ANNOUNCEMENT_TYPE_ADMIN:
        type_str = "ADMIN";
        break;
    case THORQ_PAYLOAD_ANNOUNCEMENT_TYPE_SYSTEM:
        type_str = "SYSTEM";
        break;
    }

    switch (reason) {
    case THORQ_PAYLOAD_ANNOUNCEMENT_REASON_ALERT:
        reason_str = "alert";
        break;
    case THORQ_PAYLOAD_ANNOUNCEMENT_REASON_NOTICE:
        reason_str = "notice";
        break;
    case THORQ_PAYLOAD_ANNOUNCEMENT_REASON_MAINTANENCE:
        reason_str = "maintanence";
        break;
    }

    emit Announcement(QString("[%1] %2 announcement:\n%3").arg(type_str).arg(reason_str).arg(message));
}
void Client::handleMessageEvent(std::vector<std::uint8_t> &payload)
{
	QString message;
    thorq_event_type_t type;

    thorq_payload_event_get_type(payload, type);
    thorq_payload_event_get_message(payload, message);

    switch (type) {
    case THORQ_EVENT_SESSION_REQUESTED:
		emit SessionRequested(message);
        break;
    case THORQ_EVENT_SESSION_STARTED:
        SetSessionState(THORQ_STATE_SESSION_ACTIVE);
        break;
    case THORQ_EVENT_SESSION_STOPPED:
        SetSessionState(THORQ_STATE_SESSION_NONE);
        break;
    }
}
void Client::handleMessageCommand(std::vector<std::uint8_t> &payload)
{

}
void Client::handleMessageCommandAck(std::vector<std::uint8_t> &payload)
{
	QString message;
    THORQ_PAYLOAD_ID cmd;
    THORQ_PAYLOAD_ACK result;

    thorq_payload_ack_get_id(payload, cmd);
    thorq_payload_ack_get_result(payload, result);
    thorq_payload_ack_get_message(payload, message);

    switch (result) {
    case THORQ_PAYLOAD_ACK_INVALID:
        // TODO: HMMMMMMM
        return;
    case THORQ_PAYLOAD_ACK_LOGIN_NEEDED:
        SetLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);
        return;
    case THORQ_PAYLOAD_ACK_UNAUTHORIZED:
        SetAuthState(THORQ_STATE_AUTH_NONE);
        return;
    default:
        break;
    }

    switch (cmd) {
	/*
    case THORQ_COMMAND_ID_LOGIN:
    {
        switch (result) {
        case THORQ_COMMAND_ACK_RESULT_OK:
        case THORQ_COMMAND_ACK_RESULT_NO_CHANGE:
        {
			SetUsername(message);
            SetLoginState(THORQ_STATE_LOGIN_LOGGEDIN);

            std::vector<std::uint8_t> payload;
            thorq_payload_command_pack(payload, THORQ_COMMAND_ID_GET_USER_LIST);
            SendPayload(payload, true, true);
            return;
        }
        case THORQ_COMMAND_ACK_RESULT_DENIED:
            SetUsername("");
            SetLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);
			emit Error(message);
            return;
        default:
            return;
        }
    }
    case THORQ_COMMAND_ID_LOGOUT:
    {
        switch (result) {
        case THORQ_COMMAND_ACK_RESULT_OK:
        case THORQ_COMMAND_ACK_RESULT_NO_CHANGE:
            SetUsername("");
            SetLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);
            break;
        default:
            return;
        }
	}
	*/
    case THORQ_COMMAND_ID_SESSION_REQUEST:
    {
        switch (result) {
        case THORQ_PAYLOAD_ACK_IN_PROGRESS:
            return;
        case THORQ_PAYLOAD_ACK_NO_CHANGE:
            return;
        case THORQ_PAYLOAD_ACK_DENIED:
			emit Error(message);
            return;
        default:
            return;
        }
    }
    case THORQ_COMMAND_ID_SESSION_ACCEPT:
    {
        switch (result) {
        case THORQ_PAYLOAD_ACK_OK:
            return;
        case THORQ_PAYLOAD_ACK_DENIED:
            return;
        default:
            return;
        }
    }
    case THORQ_COMMAND_ID_SESSION_DENY:
    {
        switch (result) {
        case THORQ_PAYLOAD_ACK_OK:
            return;
        case THORQ_PAYLOAD_ACK_DENIED:
			emit Error(message);
            return;
        default:
            return;
        }
    }
    case THORQ_COMMAND_ID_SESSION_LEAVE:
    {
        switch (result) {
        case THORQ_PAYLOAD_ACK_OK:
            return;
        default:
            return;
        }
    }
    default:
        return;
    }
}
void Client::handleMessageNotification(std::vector<std::uint8_t> &payload)
{
	THORQ_NOTIFICATION_TYPE type;

    thorq_payload_notification_get_type(payload, type);

	switch (type) {
	case THORQ_NOTIFICATION_USER_ACTIVITY:
	{
		QString name;
		quint8 state = 0;
        thorq_payload_notification_get_message_and_data(payload, name, state);
		emit userUpdate(name, state);
		break;
	}
	case THORQ_NOTIFICATION_USER_OFFLINE:
	case THORQ_NOTIFICATION_USER_OFFLINE_LOS:
	case THORQ_NOTIFICATION_USER_OFFLINE_TIMEOUT:
	{
		QString name;
        thorq_payload_notification_get_message(payload, name);
		emit UserOffline(name);
		break;
	}
	}
}
void Client::handleMessageCollar(std::vector<std::uint8_t> &payload)
{
	quint8 flags, shockVal, vibrateVal, beepVal, autoVal;
    thorq_payload_collar_unpack(payload, flags, shockVal, vibrateVal, beepVal, autoVal);

	if ((flags & THORQ_COLLAR_FLAG_SHOCK) != 0)
	{
		emit ReceivedShock(shockVal);
	}
	else if ((flags & THORQ_COLLAR_FLAG_VIBRATE) != 0)
	{
		emit ReceivedVibrate(vibrateVal);
	}
	else if ((flags & THORQ_COLLAR_FLAG_BEEP) != 0)
	{
		emit ReceivedBeep(beepVal);
	}
	else if ((flags & THORQ_COLLAR_FLAG_AUTO) != 0)
	{
		emit ReceivedAuto(autoVal, shockVal, vibrateVal, beepVal);
	}
}

void Client::SendPayload(std::vector<std::uint8_t>& payload, bool encrypt, bool reliable)
{
    if (encrypt)
	{
        if (!thorq_message_encode(payload, m_crypto))
			return;
	}
    else
	{
        if (!thorq_message_encode(payload))
			return;
	}

    enet_peer_send(m_peer, reliable ? 0 : 1, enet_packet_create(payload.data(), payload.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}

void Client::requestEncryptionHandshake()
{
    if (ConnectionState() != THORQ_STATE_CONNECTION_CONNECTED)
        return;

    m_crypto->reset();
    SetCryptoState(THORQ_STATE_CRYPTO_REQUESTED);

    std::vector<std::uint8_t> payload;
    thorq_payload_crypto_request_pack(payload);
    SendPayload(payload, false, true);
}

void Client::handleDisconnect(quint32 reason)
{
	m_peer = nullptr;
	SetConnectionState(THORQ_STATE_CONNECTION_DISCONNECTED);

	if (reason != 0)
	{
		switch (reason)
		{
		case THORQ_DISCONNECT_REASON_TIMEDOUT:
			emit Warning(tr("Connection timed out"));
			return;
		case THORQ_DISCONNECT_REASON_VERSION_INCOMPATIBLE:
			emit Error(tr("Please update you application\nContact the Dev:\nYameroDev#9058"));
			break;
		case THORQ_DISCONNECT_REASON_CRYPT_FAILED:
			emit Error(tr("Encryption failed"));
			break;
        case THORQ_DISCONNECT_REASON_AUTH_TIMEOUT:
			emit Error(tr("Authentication failed\nYou changed your registrationkey between pc's too quickly!\nWait a week and try again"));
            break;
        case THORQ_DISCONNECT_REASON_AUTH_REGKEY_INVALID:
			emit Error(tr("Authentication failed\nInvalid registration key!"));
			break;
        case THORQ_DISCONNECT_REASON_AUTH_SYSTEMID_BANNED:
            emit Error(tr("Authentication failed\nYou have been banned!"));
            break;
		case THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED:
			emit Warning(tr("Server shut down"));
			break;
		case THORQ_DISCONNECT_REASON_SHUTDOWN_MAINTANENCE:
			emit Warning(tr("Server is undergoing maintenance"));
			break;
		case THORQ_DISCONNECT_REASON_KICKED:
			emit Error(tr("You have been kicked"));
			break;
        case THORQ_DISCONNECT_REASON_FUCK_YOU:
            emit Error(tr("Fuck you"));
            break;
		default:
			emit Warning(tr("Disconnected for unknown reason"));
			break;
		}
	}
}
