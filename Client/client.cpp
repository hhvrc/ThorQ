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
    , m_username()
    , l_requestedPartner()
    , m_requestedPartner()
    , l_requestingPartner()
    , m_requestingPartner()
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
    , l_address()
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

int Client::ConnectionState() const
{
	return m_connectionState.load();
}

int Client::CryptoState() const
{
    return m_cryptoState.load();
}

int Client::LoginState() const
{
	return m_loginState.load();
}

int Client::SessionState() const
{
    return m_sessionState.load();
}

bool Client::Connect(const char* address, int port)
{
    SCOPELOCK(l_address);
    if (enet_address_set_host(m_address, address) < 0)
        return false;

    m_address->port = port;

    qDebug() << "Signalizing connect!";
    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_Connect);
    qDebug() << "Done.";

    return true;
}

void Client::Disconnect()
{
    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_Disconnect);
}

void Client::Login(const QString &username)
{
    SCOPELOCK(l_username);

	if (LoginState() != THORQ_LOGIN_STATE_LOGGEDOUT)
        return;

    m_username = username.toStdString();

    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_Login);
}

void Client::Logout()
{
    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_Logout);
}

void Client::RequestSession(const QString &username)
{
    SCOPELOCK(l_requestedPartner);
    m_requestedPartner = username.toStdString();
    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_SessionRequest);
}

void Client::AcceptRequest()
{
    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_SessionAccept);
}

void Client::DenyRequest()
{
    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_SessionDeny);
}

void Client::LeaveSession()
{
    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_SessionLeave);
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
		m_collarFlags.fetch_and(std::uint8_t(~THORQ_COLLAR_STATE_SHOCK));
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
		m_collarFlags.fetch_and(std::uint8_t(~THORQ_COLLAR_STATE_VIBRATE));
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
		m_collarFlags.fetch_and(std::uint8_t(~THORQ_COLLAR_STATE_BEEP));
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
		m_collarFlags.fetch_and(std::uint8_t(~THORQ_COLLAR_STATE_AUTO));
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

        // Send stuff
		if (ConnectionState() == THORQ_CONNECTION_STATE_CONNECTED)
        {
			if (SessionState() == THORQ_SESSION_STATE_ACTIVE)
            {
                if ((m_collarFlags.load() & ThorQ::CollarFlags::COLLAR_Shock) != 0)
                {
                    SendEncrypted(ThorQ::MessageContentEnums::COMMAND_Shock, true);
                }

                if ((m_collarFlags.load() & ThorQ::CollarFlags::COLLAR_Vibrate) != 0)
                {
                    SendEncrypted(ThorQ::MessageContentEnums::COMMAND_Vibrate, true);
                }

                if ((m_collarFlags.load() & ThorQ::CollarFlags::COLLAR_Beep) != 0)
                {
                    SendEncrypted(ThorQ::MessageContentEnums::COMMAND_Beep, true);
                }

                if ((m_collarFlags.load() & ThorQ::CollarFlags::COLLAR_Auto) != 0)
                {
                    SendEncrypted(ThorQ::MessageContentEnums::COMMAND_Auto, false);
                }

                if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_SessionLeave) != 0)
                {
                    SendEncrypted(ThorQ::MessageContentEnums::SESSION_Leave);
                }
            }
            else if (!m_requestingPartner.empty())
            {
                if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_SessionAccept) != 0)
                {
                    SendEncrypted(ThorQ::MessageContentEnums::SESSION_Accept, m_requestingPartner);
                    m_requestingPartner.clear();
                }
                else if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_SessionDeny) != 0)
                {
                    SendEncrypted(ThorQ::MessageContentEnums::SESSION_Deny, m_requestingPartner);
                    m_requestingPartner.clear();
                }
            }
            else if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_Disconnect) != 0)
            {
				SetConnectionState(THORQ_CONNECTION_STATE_DISCONNECTING);
                enet_peer_disconnect(m_peer, 0);
            }

			if (m_pingTimer->elapsed() > 500)
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
            if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_Connect) != 0)
            {
                qDebug() << "Connecting!";
                m_actionFlags.fetch_and(std::uint8_t(~ThorQ::ClientActionFlag::ACTION_Connect));
                m_peer = enet_host_connect(m_host, m_address, 4, 0);
				SetConnectionState(ThorQ::ConnectionState::Connecting);
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
void Client::SetConnectionState(int newState)
{
	int oldState = m_connectionState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetCryptoState(THORQ_CRYPTO_STATE_NONE);

		emit ConnectionStateChanged(newState);
    }
}
void Client::SetCryptoState(int newState)
{
	int oldState = m_cryptoState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetLoginState(THORQ_LOGIN_STATE_LOGGEDOUT);

		emit CryptoStateChanged(newState);
    }
}
void Client::SetLoginState(int newState)
{
	int oldState = m_loginState.exchange(newState);

	if (newState != oldState)
	{
		if (newState < oldState)
			SetSessionState(THORQ_SESSION_STATE_NONE);

		emit LoginStateChanged(newState);
	}
}
void Client::SetSessionState(int newState)
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

	if (ConnectionState() == ThorQ::ConnectionState::Disconnecting || ConnectionState() == ThorQ::ConnectionState::Disconnected || packet->dataLength < sizeof(std::uint8_t))
        return;

    std::uint8_t header = static_cast<std::uint8_t>(*packet->data);
    std::uint8_t* data = packet->data + sizeof(std::uint8_t);
    std::size_t size = packet->dataLength - sizeof(std::uint8_t);

    switch (header) {
    case ThorQ::MessageHeaderEnums::HEADER_HEARTBEAT:
    {
        if (m_awaitingPing)
        {
            qDebug() << "RX!";
            m_awaitingPing = false;
            SetPing(time);
        }
        return;
    }
    case ThorQ::MessageHeaderEnums::HEADER_CRYPT_ESTABLISH:
    {
        if (packet->dataLength > sizeof(std::uint8_t))
        {
            qDebug() << "Establishing!";
            if (m_crypto->IsCryptoReady())
            {
                m_crypto->Reset();
            }

            if (m_crypto->Agree(data, size))
            {
                SetCryptoState(ThorQ::CryptoState::Establishing);
                SendRaw(m_crypto->PublicKey());
            }
            else
            {
                m_crypto->Reset();
                SetCryptoState(ThorQ::CryptoState::None);
            }
        }
        return;
    }
    case ThorQ::MessageHeaderEnums::HEADER_CRYPT_VERIFY:
    {
        if (m_crypto->IsCryptoReady())
        {
            qDebug() << "Verifying!";
            SetCryptoState(ThorQ::CryptoState::Verifying);
            SendEncrypted(m_crypto->Decrypt(data, size));
        }
        return;
    }
    case ThorQ::MessageHeaderEnums::HEADER_CRYPT_OK:
    {
        SetCryptoState(ThorQ::CryptoState::Ok);
		SetConnectionState(ThorQ::ConnectionState::Connected);
        break;
    }
    default:
    {
        qDebug() << "Got unknown HeaderEnum:" << header;
        SetCryptoState(ThorQ::CryptoState::None);
		SetConnectionState(ThorQ::ConnectionState::Connecting);
        requestEncryptionHandshake();
        return;
    }
    }

    if (!m_crypto->IsCryptoReady())
        return;

    std::vector<std::uint8_t> vec = m_crypto->Decrypt(data, size);

    std::uint8_t meta = static_cast<std::uint8_t>(*vec.data());

    qDebug() << "uwu";
}

std::uint8_t Client::GetFlag()
{
    switch (CryptoState()) {
    case ThorQ::CryptoState::Requesting:
        return ThorQ::MessageHeaderEnums::HEADER_CRYPT_REQUEST;
        break;
    case ThorQ::CryptoState::Establishing:
        return ThorQ::MessageHeaderEnums::HEADER_CRYPT_ESTABLISH;
        break;
    case ThorQ::CryptoState::Verifying:
        return ThorQ::MessageHeaderEnums::HEADER_CRYPT_VERIFY;
        break;
    case ThorQ::CryptoState::Ok:
        return ThorQ::MessageHeaderEnums::HEADER_CRYPT_OK;
        break;
    }

    return 0;
}

void Client::requestEncryptionHandshake()
{
	if (ConnectionState() != THORQ_CONNECTION_STATE_CONNECTED)
        return;

    qDebug() << "Requesting!";
    m_crypto->Reset();
	SetCryptoState(THORQ_CRYPTO_STATE_REQUESTING);
    std::uint8_t flag = GetFlag();
    enet_peer_send(m_peer, 0, enet_packet_create(&flag, sizeof(std::uint8_t), ENET_PACKET_FLAG_RELIABLE));
}

void Client::SendHeartbeat()
{
    qDebug() << "TX!";

    std::uint8_t flag = ThorQ::MessageHeaderEnums::HEADER_HEARTBEAT;
    enet_peer_send(m_peer, 1, enet_packet_create(&flag, sizeof(std::uint8_t), ENET_PACKET_FLAG_UNSEQUENCED));
}

void Client::SendRaw(std::uint32_t meta, bool unreliable)
{
    meta = htonl(meta);
    SendRaw(reinterpret_cast<std::uint8_t*>(&meta), sizeof(std::uint32_t), unreliable);
}
void Client::SendRaw(std::uint32_t meta, const std::string &message, bool unreliable)
{
    meta = htonl(meta);
    std::vector<std::uint8_t> data;
    data.resize(sizeof(std::uint32_t) + message.length());
    memcpy(data.data(), &meta, sizeof(std::uint32_t));
    memcpy(data.data() + sizeof(std::uint32_t), message.data(), message.length());
    SendRaw(data, unreliable);
}
void Client::SendRaw(const std::vector<uint8_t>& data, bool unreliable)
{
    std::size_t size = data.size() + 1;
    std::uint8_t* dat = new std::uint8_t[size];
    dat[0] = GetFlag();
    memcpy(dat + 1, data.data(), data.size());

    enet_peer_send(m_peer, unreliable ? 1 : 0, enet_packet_create(dat, size, unreliable ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE));
}
void Client::SendRaw(const std::uint8_t* data, std::size_t len, bool unreliable) { SendRaw(std::vector<std::uint8_t>(data, data + len), unreliable); }

void Client::SendEncrypted(std::uint32_t meta, bool unreliable)
{
    std::uint8_t data[sizeof(std::uint8_t) + sizeof(std::uint32_t)];

    data[0] = GetFlag();

    meta = htonl(meta);
    memcpy(data + sizeof(std::uint8_t), &meta, sizeof(std::uint32_t));

    SendEncrypted(reinterpret_cast<std::uint8_t*>(&meta), sizeof(std::uint32_t), unreliable);
}
void Client::SendEncrypted(std::uint32_t meta, const std::string &message, bool unreliable)
{
    std::size_t size = sizeof(std::uint32_t) + message.length();
    std::uint8_t* data = new std::uint8_t[size];

    meta = htonl(meta);
    memcpy(data, &meta, sizeof(std::uint32_t));

    memcpy(data + sizeof(std::uint32_t), message.data(), message.length());

    SendEncrypted(data, unreliable);
}
void Client::SendEncrypted(const std::vector<std::uint8_t>& data, bool unreliable) { SendEncrypted(data.data(), data.size(), unreliable); }
void Client::SendEncrypted(const std::uint8_t* data, std::size_t len, bool unreliable)
{
    std::vector<std::uint8_t> encrypted = m_crypto->Encrypt(data, len);

    encrypted.insert(encrypted.begin(), GetFlag());

    enet_peer_send(m_peer, unreliable ? 1 : 0, enet_packet_create(encrypted.data(), encrypted.size(), unreliable ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE));
}
