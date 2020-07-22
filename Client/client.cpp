#include "client.h"

#define ENET_IMPLEMENTATION
#include <enet.h>

#include <QDebug>
#include <QThread>
#include <QElapsedTimer>

#include <enums.h>
#include <crypto.h>

#define DISCONNECT_ERROR 0x00000001
#define DISCONNECT_SHUTDOWN 0x00000002

#define SCOPELOCK(l) std::scoped_lock<std::mutex> lock(const_cast<std::mutex&>(l))

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
	, m_actionFlags(0)
	, m_collarFlags(0)
	, m_shockValue(0)
	, m_vibrateValue(0)
	, m_beepValue(0)
	, m_autoSensitivity(0)
	, m_autoShock(0)
	, m_autoVibrate(0)
	, m_autoBeep(0)
	, m_thread(new QThread())
	, m_awaitingPing(false)
	, m_pingTimer(new QElapsedTimer())
	, m_host(host)
	, m_peer(nullptr)
	, l_requestedHost()
	, m_requestedHostName("")
	, m_requestedHostPort(0)
	, m_address(new ENetAddress())
{
	QObject::connect(this, &Client::PortChanged, this, &Client::Disconnect);
	QObject::connect(this, &Client::AddressChanged, this, &Client::Disconnect);
	QObject::connect(m_thread, &QThread::started, this, &Client::Run);

	reinterpret_cast<QObject*>(this)->moveToThread(m_thread);
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

	if (m_host != nullptr)
	{
		enet_host_destroy(m_host);
	}

	delete m_address;
	delete m_pingTimer;
	delete m_crypto;
}

QString Client::Version()
{
	return QString("ENet-%1.%2.%3").arg(ENET_VERSION_MAJOR).arg(ENET_VERSION_MINOR).arg(ENET_VERSION_PATCH);
}

int Client::Ping() const
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

thorq_login_state_t Client::LoginState() const
{
	return m_loginState.load();
}

thorq_session_state_t Client::SessionState() const
{
	return m_sessionState.load();
}

void Client::Connect(const char* address, int port)
{
	{
		SCOPELOCK(l_requestedHost);
		m_requestedHostName = address;
		m_requestedHostPort = port;
	}

	m_actionFlags.fetch_or(ACTION_WantConnected);
}

void Client::Reconnect()
{

}

void Client::Disconnect()
{
	m_actionFlags.fetch_and(~ACTION_WantConnected);
}

void Client::Login(const QString &username)
{
	if (LoginState() != THORQ_LOGIN_STATE_LOGGEDOUT)
		return;

	{
		SCOPELOCK(l_username);
		m_username = username.toStdString();
	}

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

void Client::SetShock(bool enable, int strength)
{
	if (enable)
	{
		if (strength != -1)
			m_shockValue.store(strength);
		m_collarFlags.fetch_or(THORQ_COLLAR_STATE_SHOCK);
	}
	else
	{
		m_collarFlags.fetch_and(~THORQ_COLLAR_STATE_SHOCK);
	}
}
void Client::SetVibrate(bool enable, int strength)
{
	if (enable)
	{
		if (strength != -1)
			m_vibrateValue.store(strength);
		m_collarFlags.fetch_or(THORQ_COLLAR_STATE_VIBRATE);
	}
	else
	{
		m_collarFlags.fetch_and(~THORQ_COLLAR_STATE_VIBRATE);
	}
}
void Client::SetBeep(bool enable, int count)
{
	if (enable)
	{
		if (count != -1)
			m_beepValue.store(count);
		m_collarFlags.fetch_or(THORQ_COLLAR_STATE_BEEP);
	}
	else
	{
		m_collarFlags.fetch_and(~THORQ_COLLAR_STATE_BEEP);
	}
}
void Client::SetAuto(bool enable, int sensitivity, int shockStrength, int vibrateStrength, int beepCount)
{
	if (enable)
	{
		m_autoSensitivity.store(sensitivity);
		m_autoShock.store(shockStrength);
		m_autoShock.store(vibrateStrength);
		m_autoShock.store(beepCount);
		m_collarFlags.fetch_or(THORQ_COLLAR_STATE_AUTO);
	}
	else
	{
		m_collarFlags.fetch_and(~THORQ_COLLAR_STATE_AUTO);
	}
}

void Client::Run()
{
	ENetEvent event;

	while (!m_thread->isInterruptionRequested())
	{
		while (enet_host_service(m_host, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				qDebug() << "Connected!";
				SetConnectionState(THORQ_CONNECTION_STATE_CONNECTING);
				requestEncryptionHandshake();
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				HandleMessage(event.packet);
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				m_peer = nullptr;
				if (ConnectionState() != THORQ_CONNECTION_STATE_DISCONNECTING)
					emit Error("Unexpected disconnect!");
				SetConnectionState(THORQ_CONNECTION_STATE_DISCONNECTED);
				break;
			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				m_peer = nullptr;
				emit Error("Timed out!");
				SetConnectionState(THORQ_CONNECTION_STATE_DISCONNECTED);
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
			if (SessionState() == THORQ_SESSION_STATE_ACTIVE)
			{
				SendEncrypted(m_collarFlags.load(), true);
			}
			else if (SessionState() == THORQ_SESSION_STATE_DECIDING)
			{
				if ((actions & ACTION_SessionAccept) != 0)
				{
					SendEncrypted(THORQ_CMD_SESSION_ACCEPT, m_requestingPartner);
					m_requestingPartner.clear();
				}
				else if ((actions & ACTION_SessionDeny) != 0)
				{
					SendEncrypted(THORQ_CMD_SESSION_DENY, m_requestingPartner);
					m_requestingPartner.clear();
				}
			}

			if ((actions & ACTION_ReConnect) != 0 || (actions & ACTION_WantConnected) == 0)
			{
				if ((actions & ACTION_WantConnected) == 0)
					qDebug() << "Disconnecting!";
				else
					qDebug() << "Reconnecting!";

				SetConnectionState(THORQ_CONNECTION_STATE_DISCONNECTING);
				enet_peer_disconnect(m_peer, 0);
			}
			else if (m_pingTimer->elapsed() > 500)
			{
				if (m_awaitingPing)
					qDebug() << "ping timed out!";

				m_pingTimer->start();
				SendHeartbeat();
				m_awaitingPing = true;
			}
		}
		else if (ConnectionState() == THORQ_CONNECTION_STATE_DISCONNECTED)
		{
			if ((actions & ACTION_WantConnected) != 0)
			{
				qDebug() << "Connecting!";

				bool success = false;

				{
					SCOPELOCK(l_requestedHost);
					if (enet_address_set_host(m_address, m_requestedHostName.c_str()) < 0)
					{
						m_address->port = m_requestedHostPort;
						success = true;
					}
				}

				if (success)
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

	if (m_peer != nullptr)
	{
		enet_peer_disconnect_now(m_peer, DISCONNECT_SHUTDOWN);
	}
}

void Client::SetPing(int ping)
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
			SetLoginState(THORQ_LOGIN_STATE_LOGGEDOUT);

		emit CryptoStateChanged(newState);
	}
}
void Client::SetLoginState(thorq_login_state_t newState)
{
	int oldState = m_loginState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetSessionState(THORQ_SESSION_STATE_NONE);

		emit LoginStateChanged(newState);
	}
}
void Client::SetSessionState(thorq_session_state_t newState)
{
	int oldState = m_sessionState.exchange(newState);

	if (newState != oldState)
	{
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
	int time = m_pingTimer->elapsed();

	if (ConnectionState() != THORQ_CONNECTION_STATE_CONNECTED || packet->dataLength < sizeof(std::uint8_t))
		return;

	th
    ThorQ::Message msg = ThorQ::Message::Deserialize(packet->data, packet->dataLength);

    if (msg.IsHeartbeat())
    {
        if (m_awaitingPing)
        {
            qDebug() << "RX!";
            m_awaitingPing = false;
            SetPing(time);
        }
        return;
    }

    if (!msg.IsEncrypted())
    {
        if (msg.isEmpty() || msg.Meta() != ThorQ::MessageContentEnums::CRYPT_ESTABLISH)
            return;

        qDebug() << "Establishing!";

        if (m_crypto->ready())
            m_crypto->reset();

        if (m_crypto->agree(msg.payload()))
        {
            SetCryptoState(ThorQ::CryptoState::Establishing);

            msg.SetPayload(m_crypto->publicKey());
            msg.Send(m_peer);
        }
        else
        {
            m_crypto->reset();
            SetCryptoState(ThorQ::CryptoState::None);
        }
        return;
    }
    else
    {
        if (!m_crypto->ready() || !msg.Decrypt(m_crypto))
            return;

        if (msg.Meta() == ThorQ::MessageContentEnums::CRYPT_VERIFY)
        {
            qDebug() << "Verifying!";
            SetCryptoState(ThorQ::CryptoState::Verifying);

            msg.Encrypt(m_crypto);
            msg.Send(m_peer);
            return;
        }
        else if (msg.Meta() == ThorQ::MessageContentEnums::CRYPT_OK)
        {
            SetCryptoState(ThorQ::CryptoState::Ok);
            SetClientState(ThorQ::ClientState::Connected);
            return;
        }

        if (CryptoState() != ThorQ::CryptoState::Ok)
            return;
    }


    qDebug() << "uwu";
}

void Client::requestEncryptionHandshake()
{
    if (ClientState() != ThorQ::ClientState::Connecting)
        return;

    qDebug() << "Requesting!";

    m_crypto->reset();
    SetCryptoState(ThorQ::CryptoState::Requesting);

    ThorQ::Message msg;
    msg.SetMeta(ThorQ::MessageContentEnums::CRYPT_REQUEST);
    msg.Send(m_peer);
}

