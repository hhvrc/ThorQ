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

QString ipv6_to_str(const struct in6_addr& addr)
{
	char buffer[50];
	snprintf(buffer, sizeof(buffer), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
				 (int)addr.s6_addr[0], (int)addr.s6_addr[1],
				 (int)addr.s6_addr[2], (int)addr.s6_addr[3],
				 (int)addr.s6_addr[4], (int)addr.s6_addr[5],
				 (int)addr.s6_addr[6], (int)addr.s6_addr[7],
				 (int)addr.s6_addr[8], (int)addr.s6_addr[9],
				 (int)addr.s6_addr[10], (int)addr.s6_addr[11],
				 (int)addr.s6_addr[12], (int)addr.s6_addr[13],
				 (int)addr.s6_addr[14], (int)addr.s6_addr[15]);
	return QString(buffer);
}

std::string ExtractString(const std::uint8_t* data, std::size_t dataSize, std::size_t startOffset = 0, std::size_t endOffset = 0)
{
	return std::string(data + startOffset, data + dataSize - endOffset);
}

Client::Client(ENetHost* host)
    : QObject()
    , m_crypto(new ThorQ::Crypto())
    , m_clientState(ThorQ::ClientState::Disconnected)
    , m_cryptoState(ThorQ::CryptoState::None)
    , m_sessionState(ThorQ::SessionState::LoggedOut)
    , m_ping(0)
    , m_username()
    , m_partnerName()
    , m_onlineUsers()
    , m_thread(new QThread())
    , m_awaitingPing(false)
    , m_pingTimer(new QElapsedTimer())
    , m_host(host)
    , m_peer(nullptr)
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

int Client::ClientState() const
{
    return m_clientState.load();
}

int Client::CryptoState() const
{
    return m_cryptoState.load();
}

int Client::SessionState() const
{
    return m_sessionState.load();
}

QString Client::Username() const
{
    SCOPELOCK(l_username);
    return QString::fromStdString(m_username);
}

UserData Client::Partner() const
{
    SCOPELOCK(l_onlineUsers);
    for (UserData user : m_onlineUsers)
    {
        if (user.username == m_partnerName)
            return user;
    }

    return UserData();
}

QList<UserData> Client::OnlineUsers() const
{
    SCOPELOCK(l_onlineUsers);
    return m_onlineUsers;
}

void Client::Connect(const char* address, int port)
{
    SCOPELOCK(l_address);
    enet_address_set_host(m_address, address);
    m_address->port = port;

    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_Connect);
}

void Client::Disconnect()
{
    m_actionFlags.fetch_or(ThorQ::ClientActionFlag::ACTION_Disconnect);
}

void Client::Login(const QString &username)
{
    SCOPELOCK(l_username);

    if (ClientState() != ThorQ::ClientState::Connected || SessionState() != ThorQ::SessionState::LoggedOut)
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
        m_shockValue.store(strength);
        m_actionFlags.fetch_or(ThorQ::CollarFlags::COLLAR_Shock);
    }
    else
    {
        m_actionFlags.fetch_and(~ThorQ::CollarFlags::COLLAR_Shock);
    }
}
void Client::SetVibrate(bool enable, int strength)
{
    if (enable)
    {
        m_vibrateValue.store(strength);
        m_actionFlags.fetch_or(ThorQ::CollarFlags::COLLAR_Vibrate);
    }
    else
    {
        m_actionFlags.fetch_and(~ThorQ::CollarFlags::COLLAR_Vibrate);
    }
}
void Client::SetBeep(bool enable, int count)
{
    if (enable)
    {
        m_beepValue.store(count);
        m_actionFlags.fetch_or(ThorQ::CollarFlags::COLLAR_Beep);
    }
    else
    {
        m_actionFlags.fetch_and(~ThorQ::CollarFlags::COLLAR_Beep);
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
        m_actionFlags.fetch_or(ThorQ::CollarFlags::COLLAR_Auto);
    }
    else
    {
        m_actionFlags.fetch_and(~ThorQ::CollarFlags::COLLAR_Auto);
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
                SetCryptoState(ThorQ::CryptoState::None);
                SetClientState(ThorQ::ClientState::Connecting);
				break;
            case ENET_EVENT_TYPE_RECEIVE:
                HandleMessage(event.packet);
				enet_packet_destroy(event.packet);
				break;
            case ENET_EVENT_TYPE_DISCONNECT:
                SetCryptoState(ThorQ::CryptoState::None);
                SetClientState(ThorQ::ClientState::Disconnected);
                m_peer = nullptr;

                if (ClientState() != ThorQ::ClientState::Disconnecting)
                    emit Error("Unexpected disconnect!");

                SetClientState(ThorQ::ClientState::Disconnected);
				break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                SetCryptoState(ThorQ::CryptoState::None);
                SetClientState(ThorQ::ClientState::Disconnected);
                m_peer = nullptr;

                emit Error("Timed out!");

                SetClientState(ThorQ::ClientState::Disconnected);
				break;
			case ENET_EVENT_TYPE_NONE:
				break;
			}
        }

        // Send stuff
        if (ClientState() == ThorQ::ClientState::Connected)
        {
            if (SessionState() == ThorQ::SessionState::InSession)
            {
                if ((m_collarFlags.load() & ThorQ::CollarFlags::COLLAR_Shock) != 0)
                {
                    SendEncrypted(ThorQ::MessageEnums::COMMAND_Shock, true);
                }

                if ((m_collarFlags.load() & ThorQ::CollarFlags::COLLAR_Vibrate) != 0)
                {
                    SendEncrypted(ThorQ::MessageEnums::COMMAND_Vibrate, true);
                }

                if ((m_collarFlags.load() & ThorQ::CollarFlags::COLLAR_Beep) != 0)
                {
                    SendEncrypted(ThorQ::MessageEnums::COMMAND_Beep, true);
                }

                if ((m_collarFlags.load() & ThorQ::CollarFlags::COLLAR_Auto) != 0)
                {
                    SendEncrypted(ThorQ::MessageEnums::COMMAND_Auto, false);
                }

                if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_SessionLeave) != 0)
                {
                    SendEncrypted(ThorQ::MessageEnums::SESSION_Leave);
                }
            }
            else if (!m_requestingPartner.empty())
            {
                if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_SessionAccept) != 0)
                {
                    SendEncrypted(ThorQ::MessageEnums::SESSION_Accept, m_requestingPartner);
                    m_requestingPartner.clear();
                }
                else if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_SessionDeny) != 0)
                {
                    SendEncrypted(ThorQ::MessageEnums::SESSION_Deny, m_requestingPartner);
                    m_requestingPartner.clear();
                }
            }
            else
            {

            }
        }
        else if (ClientState() == ThorQ::ClientState::Disconnected)
        {
            if ((m_actionFlags.load() & ThorQ::ClientActionFlag::ACTION_Connect) != 0)
            {
                m_peer = enet_host_connect(m_host, m_address, 4, 0);
                SetClientState(ThorQ::ClientState::Connecting);
            }
        }

        if (ClientState() == ThorQ::ClientState::Connecting || ClientState() == ThorQ::ClientState::Connected)
        {
            if (m_pingTimer->elapsed() > 500)
            {
                //if (m_awaitingPing)
                //    qDebug() << "ping timed out!";

                m_pingTimer->start();
                SendHeartbeat();
                m_awaitingPing = true;
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

void Client::SetClientState(int state)
{
    if (m_clientState != state)
    {
        m_clientState = state;
        emit ClientStateChanged(state);
    }
}

void Client::SetCryptoState(int state)
{
    if (m_cryptoState != state)
    {
        m_cryptoState = state;
        emit CryptoStateChanged(state);
    }
}

void Client::SetSessionState(int state)
{
    if (m_sessionState != state)
    {
        m_sessionState = state;
        emit SessionStateChanged(state);
    }
}

void Client::SetUsername(const QString &username)
{
    SCOPELOCK(l_username);
    m_username = username.toStdString();
}

void Client::SetPartner(const QString &username)
{
    SCOPELOCK(l_onlineUsers);
    m_partnerName = username;
}

void Client::UpsertOnlineUser(const UserData &user)
{
    SCOPELOCK(l_onlineUsers);
    for (int i = 0; i < m_onlineUsers.size(); i++)
    {
        if (m_onlineUsers[i].username == user.username)
        {
            m_onlineUsers[i].hasCollar = user.hasCollar;
            m_onlineUsers[i].inSession = user.inSession;
            return;
        }
    }
    m_onlineUsers.push_back(user);
}

void Client::RemoveOnlineUser(const QString &user)
{

}

void Client::HandleMessage(ENetPacket* packet)
{
    if (ClientState() == ThorQ::ClientState::Disconnecting || ClientState() == ThorQ::ClientState::Disconnected || packet->dataLength < sizeof(std::uint8_t))
        return;

    std::uint8_t flag = ntohl(static_cast<std::uint8_t>(*packet->data));

    if ((flag & ThorQ::PreEncryptionFlag::HEARTBEAT) != 0)
    {
        if (m_awaitingPing)
        {
            SetPing(m_pingTimer->elapsed());
            m_awaitingPing = false;
        }
        return;
    }

    if (packet->dataLength == sizeof(std::uint8_t))
        return;

    std::vector<std::uint8_t> data(packet->data + sizeof(std::uint8_t), packet->data + packet->dataLength - sizeof(std::uint8_t));

    if ((flag & ThorQ::PreEncryptionFlag::CRYPT_ESTABLISH) != 0)
    {
        if (m_crypto->IsCryptoReady())
        {
            m_crypto->Reset();
        }

        if (m_crypto->Agree(data))
		{
            SendRaw(m_crypto->PublicKey());
            SetCryptoState(ThorQ::CryptoState::Establishing);
		}
		else
		{
            m_crypto->Reset();
            SetCryptoState(ThorQ::CryptoState::None);
		}
        return;
	}

    if (!m_crypto->IsCryptoReady())
        return;

    if ((flag & ThorQ::PreEncryptionFlag::CRYPT_VERIFY) != 0)
    {
        if (data.size() != sizeof(std::uint64_t))
            return;

        if (static_cast<std::uint64_t>(*data.data()) == UINT64_MAX)
        {
            SendEncrypted(data);
            SetCryptoState(ThorQ::CryptoState::Verifying);
        }
        else
        {
            m_crypto->Reset();
            SetCryptoState(ThorQ::CryptoState::None);
        }
        return;
    }

    if ((flag & ThorQ::PreEncryptionFlag::CRYPT_OK) == 0)
        return;

    SetCryptoState(ThorQ::CryptoState::Ok);
    SetClientState(ThorQ::ClientState::Connected);

    qDebug() << "uwu";
}

std::uint8_t Client::GetFlag(bool withHeartbeat)
{
    std::uint8_t flag = withHeartbeat ? ThorQ::PreEncryptionFlag::HEARTBEAT : 0;

    switch (CryptoState()) {
    case ThorQ::CryptoState::Establishing:
        flag |= ThorQ::PreEncryptionFlag::CRYPT_ESTABLISH;
        break;
    case ThorQ::CryptoState::Verifying:
        flag |= ThorQ::PreEncryptionFlag::CRYPT_VERIFY;
        break;
    case ThorQ::CryptoState::Ok:
        flag |= ThorQ::PreEncryptionFlag::CRYPT_OK;
        break;
    }

    return htonl(flag);
}

void Client::SendHeartbeat()
{
    std::uint8_t flag = GetFlag(true);
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
    if (ClientState() < ThorQ::ClientState::Connecting)
        return;

    std::size_t size = data.size() + 1;
    std::uint8_t* dat = new std::uint8_t[size];
    dat[0] = GetFlag();
    memcpy(dat + 1, data.data(), data.size());

    enet_peer_send(m_peer, unreliable ? 1 : 0, enet_packet_create(dat, size, unreliable ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE));
}
void Client::SendRaw(const std::uint8_t* data, std::size_t len, bool unreliable) { SendRaw(std::vector<std::uint8_t>(data, data + len), unreliable); }

void Client::SendEncrypted(std::uint32_t meta, bool unreliable)
{
    meta = htonl(meta);
    SendEncrypted(reinterpret_cast<std::uint8_t*>(&meta), sizeof(std::uint32_t), unreliable);
}
void Client::SendEncrypted(std::uint32_t meta, const std::string &message, bool unreliable)
{
    meta = htonl(meta);
    std::vector<std::uint8_t> data;
    data.resize(sizeof(std::uint32_t) + message.length());
    memcpy(data.data(), &meta, sizeof(std::uint32_t));
    memcpy(data.data() + sizeof(std::uint32_t), message.data(), message.length());
    SendEncrypted(data, unreliable);
}
void Client::SendEncrypted(const std::vector<std::uint8_t>& data, bool unreliable) { SendEncrypted(data.data(), data.size(), unreliable); }
void Client::SendEncrypted(const std::uint8_t* data, std::size_t len, bool unreliable)
{
    if (ClientState() != ThorQ::ClientState::Connected)
        return;

    std::vector<std::uint8_t> encrypted = m_crypto->Encrypt(data, len);

    encrypted.insert(encrypted.begin(), GetFlag());

    enet_peer_send(m_peer, unreliable ? 1 : 0, enet_packet_create(encrypted.data(), encrypted.size(), unreliable ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE));
}
