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
#include <thorq_payload_auth.h>
#include <thorq_payload_collar.h>
#include <thorq_payload_command.h>
#include <thorq_payload_version.h>
#include <thorq_payload_heartbeat.h>

#define DISCONNECT_ERROR 0x00000001
#define DISCONNECT_SHUTDOWN 0x00000002

#define SCOPELOCK(l) std::scoped_lock lock(const_cast<std::mutex&>(l))

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
	, m_connectionState(THORQ_CONNECTION_STATE_DISCONNECTED)
	, m_cryptoState(THORQ_CRYPTO_STATE_NONE)
	, m_authState(THORQ_AUTH_STATE_NONE)
	, m_loginState(THORQ_LOGIN_STATE_LOGGEDOUT)
	, m_sessionState(THORQ_SESSION_STATE_NONE)
	, m_ping(0)
	, l_username()
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
	, m_awaitingPing(false)
    , m_lastPing(0)
	, m_pingTimer(new QElapsedTimer())
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

	if (ConnectionState() != THORQ_CONNECTION_STATE_DISCONNECTED)
	{
		Disconnect();
		do { Service(); }
		while (ConnectionState() != THORQ_CONNECTION_STATE_DISCONNECTED);
	}

	if (m_host != nullptr)
	{
		enet_host_destroy(m_host);
	}

	delete m_address;
	delete m_pingTimer;
	delete m_crypto;
	delete m_thread;
}

QString Client::Version()
{
	return QString("ENet-%1.%2.%3").arg(ENET_VERSION_MAJOR).arg(ENET_VERSION_MINOR).arg(ENET_VERSION_PATCH);
}

uint Client::Ping() const
{
	return m_ping.load();
}

thorq_connection_state_t Client::ConnectionState() const
{
	return m_connectionState.load();
}

thorq_crypto_state_t Client::CryptoState() const
{
	return m_cryptoState.load();
}

thorq_auth_state_t Client::AuthState() const
{
	return m_authState.load();
}

thorq_login_state_t Client::LoginState() const
{
	return m_loginState.load();
}

thorq_session_state_t Client::SessionState() const
{
	return m_sessionState.load();
}

void Client::Connect(const char* address, std::uint16_t port)
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
	if (LoginState() != THORQ_LOGIN_STATE_LOGGEDOUT)
		return;

    SCOPELOCK(l_username);
    m_username = username.toStdString();

	m_actionFlags.fetch_or(ACTION_Login);
}

void Client::Logout()
{
	if (LoginState() != THORQ_LOGIN_STATE_LOGGEDIN)
		return;

	m_actionFlags.fetch_or(ACTION_Logout);
}

void Client::RequestSession(const QString &username)
{
	SCOPELOCK(l_requestedPartner);
	m_requestedPartner = username.toStdString();
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

void Client::SetRegistrationKey(const QString& regKey)
{
    SCOPELOCK(l_registrationKey);
    m_registrationKey = regKey.toStdString();

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
			SetConnectionState(THORQ_CONNECTION_STATE_CONNECTED);
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
	if (ConnectionState() == THORQ_CONNECTION_STATE_CONNECTED)
    {
        if (LoginState() == THORQ_LOGIN_STATE_LOGGEDIN)
        {
            if ((actions & ACTION_Logout) != 0)
            {
                thorq_payload_t payload;

                SCOPELOCK(l_username);
                thorq_payload_command_pack(payload, THORQ_COMMAND_ID_LOGIN, m_username);
                SendPayload(payload, true, true);

                SetLoginState(THORQ_LOGIN_STATE_LOGGINGIN);
            }
            else
            {
                std::uint64_t collarState = m_collarState.fetch_and(~0xFF);

                if (SessionState() == THORQ_SESSION_STATE_ACTIVE && ((collarState & THORQ_COLLAR_FLAG_IMPULSE) != 0))
                {
                    thorq_payload_t payload;
                    thorq_payload_collar_pack(payload, collarState & 0xFF, (collarState >> 56) & 0xFF, (collarState >> 48) & 0xFF, (collarState >> 40) & 0xFF, (collarState >> 32) & 0xFF);
                    SendPayload(payload, true, false);
                }
                else
                {
                    if ((actions & ACTION_SessionRequest) != 0)
                    {
                        thorq_payload_t payload;
                        thorq_payload_command_pack(payload, THORQ_COMMAND_ID_SESSION_DENY, m_requestedPartner);
                        SendPayload(payload, true, true);
                        SetSessionState(THORQ_SESSION_STATE_REQUESTING);
                    }

                    if (SessionState() == THORQ_SESSION_STATE_DECIDING)
                    {
                        if ((actions & ACTION_SessionAccept) != 0)
                        {
                            thorq_payload_t payload;
                            thorq_payload_command_pack(payload, THORQ_COMMAND_ID_SESSION_ACCEPT, m_requestingPartner);
                            SendPayload(payload, true, true);
                            m_requestingPartner.clear();
                            SetSessionState(THORQ_SESSION_STATE_JOINING);
                        }
                        else if ((actions & ACTION_SessionDeny) != 0)
                        {
                            thorq_payload_t payload;
                            thorq_payload_command_pack(payload, THORQ_COMMAND_ID_SESSION_DENY, m_requestingPartner);
                            SendPayload(payload, true, true);
                            m_requestingPartner.clear();
                            SetSessionState(THORQ_SESSION_STATE_NONE);
                        }
                    }
                }
            }
        }
        else if (LoginState() == THORQ_LOGIN_STATE_LOGGEDOUT)
        {
            if ((actions & ACTION_SendRegKey) != 0)
            {
                if (AuthState() == THORQ_AUTH_STATE_REGKEY_AWAITING_INPUT)
                {
                    thorq_payload_t payload;

                    SCOPELOCK(l_registrationKey);
                    thorq_payload_auth_pack(payload, THORQ_AUTH_REGKEY, std::vector<std::uint8_t>(m_registrationKey.begin(), m_registrationKey.end()));

                    SendPayload(payload, true, true);
                    SetAuthState(THORQ_AUTH_STATE_REGKEY_CHECKING);
                }
            }
            else if ((actions & ACTION_Login) != 0)
            {
                thorq_payload_t payload;

                SCOPELOCK(l_username);
                thorq_payload_command_pack(payload, THORQ_COMMAND_ID_LOGIN, m_username);
                SendPayload(payload, true, true);

                SetLoginState(THORQ_LOGIN_STATE_LOGGINGIN);
            }

        }


        /** Disconnects client gracefully
          * If [STATE] ACTION_Connected is not set, then the clients should not be connected
          * If [FLAG]  ACTION_ReConnect is set, then the client should disconnect, and will naturally reconnect again (i love state machines)
          */
		if ((actions & ACTION_ReConnect) != 0 || (actions & ACTION_Connected) == 0)
        {
			SetConnectionState(THORQ_CONNECTION_STATE_DISCONNECTING);
			enet_peer_disconnect(m_peer, 0);
		}
		else
		{
			std::uint64_t elapsed = m_pingTimer->elapsed();

			if ((elapsed - m_lastPing) > 500)
			{

				if (m_awaitingPing)
				{
					SetPing(elapsed);
					m_lastPing = elapsed;
				}
				else
				{
					m_pingTimer->start();
					m_lastPing = 0;
				}

				thorq_payload_t payload;
				thorq_payload_heartbeat_pack(payload);
				SendPayload(payload, false, false);
				m_awaitingPing = true;
			}
		}
	}
	else if (ConnectionState() == THORQ_CONNECTION_STATE_DISCONNECTED)
	{
		if ((actions & ACTION_Connected) != 0)
        {
            bool addressFound = false;

			{
				SCOPELOCK(l_requestedHost);
				if (enet_address_set_host(m_address, m_requestedHostName.c_str()) == 0)
				{
					m_address->port = m_requestedHostPort;
                    addressFound = true;
				}
			}

            if (addressFound)
			{
				m_peer = enet_host_connect(m_host, m_address, 4, 0);
				SetConnectionState(THORQ_CONNECTION_STATE_CONNECTING);
			}
			else
			{
				// TODO: something
			}
		}
	}
}

void Client::SetPing(std::uint16_t ping)
{
	if (m_ping != ping)
    {
        m_ping = ping;
		emit PingChanged(ping);
	}
}

// Cascading setters
void Client::SetConnectionState(thorq_connection_state_t newState)
{
	int oldState = m_connectionState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
            SetCryptoState(THORQ_CRYPTO_STATE_NONE);

		emit ConnectionStateChanged(newState);
	}
}
void Client::SetCryptoState(thorq_crypto_state_t newState)
{
	int oldState = m_cryptoState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetAuthState(THORQ_AUTH_STATE_NONE);
		else
			SetConnectionState(THORQ_CONNECTION_STATE_CONNECTED);

		emit CryptoStateChanged(newState);
	}
}

void Client::SetAuthState(thorq_auth_state_t newState)
{
	int oldState = m_authState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetLoginState(THORQ_LOGIN_STATE_LOGGEDOUT);
		else
			SetCryptoState(THORQ_CRYPTO_STATE_ACTIVE);

		emit AuthStateChanged(newState);
	}
}
void Client::SetLoginState(thorq_login_state_t newState)
{
	int oldState = m_loginState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetSessionState(THORQ_SESSION_STATE_NONE);
		else
			SetAuthState(THORQ_AUTH_STATE_OK);

		emit LoginStateChanged(newState);
	}
}
void Client::SetSessionState(thorq_session_state_t newState)
{
	int oldState = m_sessionState.exchange(newState);

	if (newState != oldState)
	{
		if (newState > oldState)
			SetLoginState(THORQ_LOGIN_STATE_LOGGEDIN);

		emit SessionStateChanged(newState);
	}
}

void Client::SetUsername(const QString &username)
{
	SCOPELOCK(l_username);
	m_username = username.toStdString();
}

void Client::SetPartner(const QString &username)
{
	SCOPELOCK(l_partnerName);
	m_partnerName = username.toStdString();
}

void Client::HandleMessage(ENetPacket* packet)
{
	if (ConnectionState() != THORQ_CONNECTION_STATE_CONNECTED || !thorq_message_is_valid(packet->data, packet->dataLength))
		return;

	std::vector<std::uint8_t> message;
	thorq_message_decode(packet->data, packet->dataLength, message, m_crypto);

	thorq_payload_t payload;
	thorq_payload_unpack(message, payload);

	switch (payload.id) {
    case THORQ_PAYLOAD_ID_INVALID:
		return;
	case THORQ_PAYLOAD_ID_VERSION:
		if (thorq_payload_version_is_valid(payload))
			handleMessageVersion(payload);
		return;
	case THORQ_PAYLOAD_ID_CRYPTO:
		if (thorq_payload_crypto_is_valid(payload))
			handleMessageCrypto(payload);
		return;
	case THORQ_PAYLOAD_ID_AUTH:
		if (thorq_payload_auth_is_valid(payload))
			handleMessageAuth(payload);
		return;
	case THORQ_PAYLOAD_ID_HEARTBEAT:
		if (thorq_payload_heartbeat_is_valid(payload))
            handleMessageHeartbeat();
		return;
	default:
		if (AuthState() != THORQ_AUTH_STATE_OK)
		{
			return;
		}

		break;
	}

	qDebug() << "UwU";
}

void Client::handleMessageVersion(const thorq_payload_t& payload)
{
	std::uint8_t app;
	thorq_version_t version;
	thorq_payload_version_unpack(payload, app, version);

	switch (app) {
	case THORQ_APP_SERVER:
		if (version > THORQ_VERSION_SERVER)
		{ qDebug() << tr("Server has updated from %1 to %2").arg(THORQ_VERSION_SERVER.to_string().c_str()).arg(version.to_string().c_str()); }
		else if (version < THORQ_VERSION_SERVER)
		{ qDebug() << tr("Server had downdated from %1 to %2").arg(THORQ_VERSION_SERVER.to_string().c_str()).arg(version.to_string().c_str()); }
		else
		{ qDebug() << tr("Server version compatible"); }
		break;
	case THORQ_APP_CLIENT:
		if (version > THORQ_VERSION_CLIENT)
		{ qDebug() << tr("Client has updated from %1 to %2").arg(THORQ_VERSION_CLIENT.to_string().c_str()).arg(version.to_string().c_str()); emit Error(tr("New update available!\nClient v%1").arg(version.to_string().c_str())); }
		else if (version < THORQ_VERSION_CLIENT)
		{ qDebug() << tr("Client has downgraded from %1 to %2").arg(THORQ_VERSION_CLIENT.to_string().c_str()).arg(version.to_string().c_str()); emit Error(tr("Hello future-person!\nServer expects: Client v%1\nYou have: Client v%2").arg(version.to_string().c_str()).arg(THORQ_VERSION_CLIENT.to_string().c_str())); }
		else
		{ qDebug() << tr("Client version compatible"); }
		break;
	case THORQ_APP_LINK:
		if (version > THORQ_VERSION_LINK)
		{ qDebug() << tr("Protocol has updated from %1 to %2").arg(THORQ_VERSION_LINK.to_string().c_str()).arg(version.to_string().c_str()); emit Error(tr("Version incompatible!\nPlease upgrade")); }
		else if (version < THORQ_VERSION_LINK)
		{ qDebug() << tr("Protocol has downgraded from %1 to %2").arg(THORQ_VERSION_LINK.to_string().c_str()).arg(version.to_string().c_str()); emit Error(tr("Version inompatible!\nPlease downgrade")); }
		else
		{ qDebug() << tr("Protocol version compatible"); }
		break;
	default:
		qDebug() << tr("Got ivalid version %1[%2]").arg(app).arg(version.to_string().c_str());
		return;
	}
}
void Client::handleMessageHeartbeat()
{
	if (m_awaitingPing)
	{
		m_awaitingPing = false;
		SetPing(m_pingTimer->elapsed());
	}
}
void Client::handleMessageCrypto(const thorq_payload_t& payload)
{
    thorq_payload_t response;

    thorq_crypto_cmd_t cmd;
    thorq_payload_crypto_get_cmd(payload, cmd);

    switch (cmd) {
	case THORQ_CRYPTO_ESTABLISH:
    {
        SetCryptoState(THORQ_CRYPTO_STATE_ESTABLISHING);

        std::vector<std::uint8_t> data;
        thorq_payload_crypto_get_data(payload, data);

        if (m_crypto->ready()) m_crypto->reset();

        if (m_crypto->agree(data))
        {
            thorq_payload_crypto_pack(response, THORQ_CRYPTO_ESTABLISH, m_crypto->publicKey());
            SendPayload(response, false, true);
		}
		else
		{
			m_crypto->reset();
			SetCryptoState(THORQ_CRYPTO_STATE_NONE);
		}
        break;
    }
    case THORQ_CRYPTO_VERIFY:
        SetCryptoState(THORQ_CRYPTO_STATE_VERIFYING);
        SendPayload(payload, true, true);
		break;
    case THORQ_CRYPTO_OK:
        SetCryptoState(THORQ_CRYPTO_STATE_ACTIVE);
		break;
	default:
        qDebug() << "CRYPT: Unexpected message:" << cmd;
		return;
	}
}
void Client::handleMessageAuth(const thorq_payload_t& payload)
{
    thorq_payload_t response;

    thorq_auth_cmd_t cmd;
    thorq_payload_auth_get_cmd(payload, cmd);

    switch (cmd) {
    case THORQ_AUTH_SYSTEMID_REQ:
        thorq_payload_auth_pack(response, THORQ_AUTH_SYSTEMID, ThorQ::systemid_generate());
        SendPayload(response);
        SetAuthState(THORQ_AUTH_STATE_HWID_CHECKING);
		break;
    case THORQ_AUTH_REGKEY_REQ:
        thorq_payload_auth_pack(response, THORQ_AUTH_REGKEY_AWAITING_INPUT);
		SendPayload(response);
        SetAuthState(THORQ_AUTH_STATE_REGKEY_AWAITING_INPUT);
		emit RequestingRegistrationKey();
		break;
	case THORQ_AUTH_OK:
		SetAuthState(THORQ_AUTH_STATE_OK);
		break;
	default:
        qDebug() << "AUTH: Unexpected message:" << cmd;
		break;
	}
}

void Client::SendPayload(const thorq_payload_t& payload, bool encrypt, bool reliable)
{
	std::vector<std::uint8_t> message;

	thorq_payload_pack(payload, message);

	std::vector<std::uint8_t> data;

    if (encrypt)
	{
        thorq_message_encode(message, data, m_crypto);
	}
    else
	{
        thorq_message_encode(message, data);
	}

	enet_peer_send(m_peer, reliable ? 0 : 1, enet_packet_create(data.data(), data.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}

void Client::requestEncryptionHandshake()
{
    if (ConnectionState() != THORQ_CONNECTION_STATE_CONNECTED)
        return;

    m_crypto->reset();
    SetCryptoState(THORQ_CRYPTO_STATE_REQUESTED);

	thorq_payload_t payload;
	thorq_payload_crypto_pack(payload, THORQ_CRYPTO_REQUEST);
	SendPayload(payload, false, true);
}

void Client::handleDisconnect(std::uint32_t reason)
{
	m_peer = nullptr;
	SetConnectionState(THORQ_CONNECTION_STATE_DISCONNECTED);

	if (reason != 0)
	{
		switch (reason)
		{
		case THORQ_DISCONNECT_REASON_TIMEDOUT:
			emit Error(tr("Connection timed out"));
			return;
		case THORQ_DISCONNECT_REASON_VERSION_INCOMPATIBLE:
			emit Error(tr("Please update you application\nDiscord: YameroDev#9058"));
			break;
		case THORQ_DISCONNECT_REASON_CRYPT_FAILED:
			emit Error(tr("Encryption failed"));
			break;
        case THORQ_DISCONNECT_REASON_AUTH_TIMEOUT:
            emit Error(tr("Authentication failed"));
            break;
        case THORQ_DISCONNECT_REASON_AUTH_INVALID_REGKEY:
			emit Error(tr("Authentication failed"));
			break;
        case THORQ_DISCONNECT_REASON_AUTH_INVALID_SYSTEMID:
            emit Error(tr("Authentication failed"));
            break;
		case THORQ_DISCONNECT_REASON_SHUTDOWN_CLOSED:
			emit Error(tr("Server shut down"));
			break;
		case THORQ_DISCONNECT_REASON_SHUTDOWN_MAINTANENCE:
			emit Error(tr("Server is undergoing maintenance"));
			break;
		case THORQ_DISCONNECT_REASON_KICKED:
			emit Error(tr("You have been kicked"));
			break;
		default:
			emit Error(tr("Disconnected for unknown reason"));
			break;
		}
	}
}
