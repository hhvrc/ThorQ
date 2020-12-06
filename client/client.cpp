#include "client.h"

#include <thread>
#include <chrono>
#include <array>

#if defined(__GCC__) || defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
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
#include <version.h>
#include <systemid.h>
#include <constants.h>
#include <thorq_message.h>

#include <flatbuffers/flatbuffers.h>
#include <schemas/message_generated.h>
#include <schemas/crypto_generated.h>
#include <schemas/account_generated.h>
#include <schemas/collar_generated.h>
#include <schemas/version_generated.h>
#include <schemas/account_generated.h>
#include <schemas/systemid_generated.h>
#include <schemas/heartbeat_generated.h>
#include <schemas/announcement_generated.h>
#include <schemas/systemid_generated.h>

#define SCOPELOCK(l) std::scoped_lock lock(const_cast<std::mutex&>(l))

/**
 * @enum THORQ_CLIENT_ACTION
 * @brief Flags to tell client how to behave and what to do
 */
enum THORQ_CLIENT_ACTION
{
    ACTION_Connect        = 1 << 0,  ///< [Toggle] Try to connect, and stay connected
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
    ACTION_TOGGLEACTIONS  = ACTION_Connect ///< All action flags that are meant to be toggled, and not used as signals, these flags will not be cleared after they are read
};

/**
 * @enum THORQ_COLLAR_FLAG
 * @brief Flags to describe current user input
 */
enum class THORQ_COLLAR_FLAG : std::uint16_t
{
    SHOCK      = 1 << 0, ///< Activate collar shock
    VIBRATE    = 1 << 1, ///< Activate collar vibration
    BEEP       = 1 << 2, ///< Activate collar speaker
    AUTO       = 1 << 3, ///< Auto mode
    RESERVED_5 = 1 << 4,
    RESERVED_6 = 1 << 5,
    RESERVED_7 = 1 << 6,
    IMPULSE    = 1 << 7,
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
    , m_crypto(std::make_shared<ThorQ::Crypto>())
	, m_connectionState(THORQ_STATE_CONNECTION_DISCONNECTED)
	, m_cryptoState(THORQ_STATE_CRYPTO_NONE)
    , m_authState(THORQ_STATE_HWID_NONE)
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
    , m_heartbeatInterval(1000)
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
    m_serviceTimer->setInterval(10);
	m_serviceTimer->start();

	m_thread->start();
}

Client* Client::NewClient()
{
    ENetHost* host = enet_host_create(nullptr, 1, (std::uint8_t)THORQ_CHANNEL::_MAX, 0, 0);

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

THORQ_STATE_HWID Client::HwidState() const
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

    m_actionFlags.fetch_or(ACTION_Connect);
}

void Client::Reconnect()
{
	m_actionFlags.fetch_or(ACTION_ReConnect);
}

void Client::Disconnect()
{
    m_actionFlags.fetch_and(~ACTION_Connect);
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
    m_collarState.fetch_or(((std::uint64_t)value << 56) | (std::uint16_t)THORQ_COLLAR_FLAG::SHOCK);
}
void Client::SetVibrate(std::uint8_t value)
{
    m_collarState.fetch_and(~((0xFFull << 48) | 0xFFull));
    m_collarState.fetch_or((std::uint64_t(value) << 48) | (std::uint16_t)THORQ_COLLAR_FLAG::VIBRATE);
}
void Client::SetBeep(std::uint8_t value)
{
    m_collarState.fetch_and(~((0xFFull << 40) | 0xFFull));
    m_collarState.fetch_or((std::uint64_t(value) << 40) | (std::uint16_t)THORQ_COLLAR_FLAG::BEEP);
}
void Client::EnableAuto(std::uint8_t value)
{
    m_collarState.fetch_and(~((0xFFull << 32) | 0xFFull));
    m_collarState.fetch_or((std::uint64_t(value) << 32) | (std::uint16_t)THORQ_COLLAR_FLAG::AUTO);
}
void Client::DisableAuto()
{
    m_collarState.fetch_and(~(std::uint16_t)THORQ_COLLAR_FLAG::AUTO);
}
void Client::SendImpulse()
{
    m_collarState.fetch_or((std::uint16_t)THORQ_COLLAR_FLAG::IMPULSE);
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
            qDebug() << "packet";
            handleMessage(event.packet);
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
            handleDisconnect((THORQ_DISCONNECT_REASON)event.data);
			break;
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
            handleDisconnect(THORQ_DISCONNECT_REASON::TIMED_OUT);
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

                flatbuffers::FlatBufferBuilder fbsBuilder;
                auto fbsLogout  = ThorQ::Serialization::Account::CreateLogout(fbsBuilder, false).Union();
                auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_logout, fbsLogout).Union();
                auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);
                fbsBuilder.Finish(fbsMessage);

                // Calculate packet size
                std::size_t size = ThorQ::calculatePacketSize(fbsBuilder.GetSize(), false);

                //
                ENetPacket* packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_RELIABLE);
                ThorQ::packetEncode(packet, std::span<std::uint8_t>(fbsBuilder.GetBufferPointer(), fbsBuilder.GetSize()));
                packetSend(packet, THORQ_CHANNEL::MAIN);
                m_awaitingHeartbeat = true;

				SetLoginState(THORQ_STATE_LOGIN_LOGGINGOUT);
            }/*
            else
            {
                std::uint64_t collarState = m_collarState.fetch_and(~0xFF);

                if (SessionState() == THORQ_STATE_SESSION_ACTIVE && ((collarState & THORQ_COLLAR_FLAG_IMPULSE) != 0))
                {
                    std::vector<std::uint8_t> payload;
                    thorq_payload_collar_pack(payload, collarState & 0xFF, (collarState >> 56) & 0xFF, (collarState >> 48) & 0xFF, (collarState >> 40) & 0xFF, (collarState >> 32) & 0xFF);
                    SendPayload(payload, THORQ_CHANNEL_IMPULSE, true, false);
                }
                else
                {
                    if ((actions & ACTION_SessionRequest) != 0)
                    {
                        std::vector<std::uint8_t> payload;
                        thorq_payload_session_pack(payload, THORQ_PAYLOAD_SESSION_REQUEST, m_requestedPartner);
                        SendPayload(payload, THORQ_CHANNEL_MAIN, true, true);
                    }
                    else if ((actions & ACTION_SessionAccept) != 0)
                    {
                        std::vector<std::uint8_t> payload;
                        thorq_payload_session_pack(payload, THORQ_PAYLOAD_SESSION_ACCEPT, m_requestedPartner);
                        SendPayload(payload, THORQ_CHANNEL_MAIN, true, true);
                        m_requestingPartner.clear();
                        SetSessionState(THORQ_STATE_SESSION_JOINING);
                    }
                    else if ((actions & ACTION_SessionDeny) != 0)
                    {
                        std::vector<std::uint8_t> payload;
                        thorq_payload_session_pack(payload, THORQ_PAYLOAD_SESSION_DENY, m_requestedPartner);
                        SendPayload(payload, THORQ_CHANNEL_MAIN, true, true);
                        m_requestingPartner.clear();
                        SetSessionState(THORQ_STATE_SESSION_NONE);
                    }
                }
            }*/
        }/*
        else if (LoginState() == THORQ_STATE_LOGIN_LOGGEDOUT)
        {
            if ((actions & ACTION_Login) != 0)
            {
                std::vector<std::uint8_t> payload;

                SCOPELOCK(l_loginInfo);
                thorq_payload_account_login_pack(payload, m_username, m_password);
                SendPayload(payload, THORQ_CHANNEL_MAIN, true, true);

                SetLoginState(THORQ_STATE_LOGIN_LOGGINGIN);
            }

        }*/

        /**
         * Disconnects client gracefully
         * If [STATE] ACTION_Connected is not set, then the clients should not be connected
         * If [FLAG]  ACTION_ReConnect is set, then the client should disconnect, and will naturally reconnect again (i love state machines)
         */
        if ((actions & ACTION_ReConnect) != 0 || (actions & ACTION_Connect) == 0)
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

                // Build flatbuffer
                flatbuffers::FlatBufferBuilder fbsBuilder;
                auto fbsHeartbeat = ThorQ::Serialization::Heartbeat::CreateMessage(fbsBuilder, Rtt()).Union();
                auto fbsMessage   = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_heartbeat, fbsHeartbeat);
                fbsBuilder.Finish(fbsMessage);

                // Calculate packet size
                std::size_t size = ThorQ::calculatePacketSize(fbsBuilder.GetSize(), false);

                //
                ENetPacket* packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_RELIABLE);
                ThorQ::packetEncode(packet, std::span<std::uint8_t>(fbsBuilder.GetBufferPointer(), fbsBuilder.GetSize()));
                packetSend(packet, THORQ_CHANNEL::MAIN);
                m_awaitingHeartbeat = true;
			}
		}
    }
	else if (ConnectionState() == THORQ_STATE_CONNECTION_DISCONNECTED)
	{
        if ((actions & ACTION_Connect) != 0)
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
            SetAuthState(THORQ_STATE_HWID_NONE);
		else
			SetConnectionState(THORQ_STATE_CONNECTION_CONNECTED);

		emit CryptoStateChanged(newState);
	}
}

void Client::SetAuthState(THORQ_STATE_HWID newState)
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
            SetAuthState(THORQ_STATE_HWID_OK);

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

void Client::handleMessage(ENetPacket* packet)
{
    if (ConnectionState() != THORQ_STATE_CONNECTION_CONNECTED ||
        !ThorQ::packetIsValidSize(packet))
    {
        return;
    }

    std::vector<std::uint8_t> message;
    message.resize(ThorQ::calculateDataSize(packet));

    if (!ThorQ::packetDecode(packet, message, m_crypto))
	{
        return;
	}

    const ThorQ::Serialization::Message* fbsMessage = flatbuffers::GetRoot<ThorQ::Serialization::Message>(message.data());
    flatbuffers::Verifier fbsVerifier(message.data(), message.size());

    if (!fbsMessage->Verify(fbsVerifier))
    {
        qDebug() << "Received invlaid buffer";
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        handleMessageAccount(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_heartbeat:
        handleMessageHeartbeat(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_crypto:
        handleMessageCrypto(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_system_id:
        handleMessageSystemID(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_group:
        handleMessageGroup(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_collar:
        handleMessageCollar(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_moderation:
        handleMessageModeration(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_friend_request:
        handleMessageFriendRequest(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_file:
        handleMessageFile(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_user:
        handleMessageUser(fbsMessage->body(), fbsVerifier);
        break;
    case ThorQ::Serialization::Body_announcement:
        handleMessageAnnouncement(fbsMessage->body(), fbsVerifier);
        break;
	default:
        qWarning() << "Invalid packet type, packet might be corrupt";
        if (HwidState() != THORQ_STATE_HWID_OK)
		{
			return;
		}

		break;
    }
}

void Client::handleMessageVersion(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsVersion = reinterpret_cast<const ThorQ::Serialization::Version*>(body);

    if (!fbsVersion->Verify(fbsVerifier))
    {
        return;
    }

    ThorQ::Version version(fbsVersion);

    switch ((THORQ_APP)fbsVersion->app()) {
    case THORQ_APP::SERVER:
        if (version > THORQ_VERSION_SERVER)
        { qDebug() << tr("Server has updated from %1 to %2").arg(THORQ_VERSION_SERVER.toString().c_str()).arg(version.toString().c_str()); }
        else if (version < THORQ_VERSION_SERVER)
        { qDebug() << tr("Server had downdated from %1 to %2").arg(THORQ_VERSION_SERVER.toString().c_str()).arg(version.toString().c_str()); }
        else
        { qDebug() << tr("Server version compatible"); }
        break;
    case THORQ_APP::CLIENT:
        if (version > THORQ_VERSION_CLIENT)
        { qDebug() << tr("Client has updated from %1 to %2").arg(THORQ_VERSION_CLIENT.toString().c_str()).arg(version.toString().c_str()); emit Error(tr("New update available!\nClient v%1").arg(version.toString().c_str())); }
        else if (version < THORQ_VERSION_CLIENT)
        { qDebug() << tr("Client has downgraded from %1 to %2").arg(THORQ_VERSION_CLIENT.toString().c_str()).arg(version.toString().c_str()); emit Error(tr("Hello future-person!\nServer expects: Client v%1\nYou have: Client v%2").arg(version.toString().c_str()).arg(THORQ_VERSION_CLIENT.toString().c_str())); }
        else
        { qDebug() << tr("Client version compatible"); }
        break;
    case THORQ_APP::LINK:
        if (version > THORQ_VERSION_LINK)
        { qDebug() << tr("Protocol has updated from %1 to %2").arg(THORQ_VERSION_LINK.toString().c_str()).arg(version.toString().c_str()); emit Error(tr("Version incompatible!\nPlease upgrade")); }
        else if (version < THORQ_VERSION_LINK)
        { qDebug() << tr("Protocol has downgraded from %1 to %2").arg(THORQ_VERSION_LINK.toString().c_str()).arg(version.toString().c_str()); emit Error(tr("Version inompatible!\nPlease downgrade")); }
        else
        { qDebug() << tr("Protocol version compatible"); }
        break;
    default:
        qDebug() << tr("Got ivalid version %1[%2]").arg(fbsVersion->app()).arg(version.toString().c_str());
        return;
    }
}
void Client::handleMessageHeartbeat(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsHeartbeat = reinterpret_cast<const ThorQ::Serialization::Heartbeat::Message*>(body);

    if (!fbsHeartbeat->Verify(fbsVerifier))
    {
        return;
    }

    if (m_awaitingHeartbeat && fbsHeartbeat->Verify(fbsVerifier))
    {
        m_awaitingHeartbeat = false;

        // Set interval from server
        m_heartbeatInterval = fbsHeartbeat->interval();
        SetRtt(m_heartbeatTimer->elapsed());
    }
}

void Client::handleMessageUser(const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsUser = reinterpret_cast<const ThorQ::Serialization::User::Message*>(body);

    if (!fbsUser->Verify(fbsVerifier))
    {
        return;
    }
}

void Client::handleMessageFile(const void *body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsFile = reinterpret_cast<const ThorQ::Serialization::File::Message*>(body);

    if (!fbsFile->Verify(fbsVerifier))
    {
        return;
    }
}

void Client::handleMessageCrypto(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(body);

    if (!fbsCrypto->Verify(fbsVerifier))
    {
        return;
    }

    switch (fbsCrypto->type()) {
    case ThorQ::Serialization::Crypto::MessageType_Establish:
    {
        qDebug() << "CRYPTO: establish";
        SetCryptoState(THORQ_STATE_CRYPTO_ESTABLISHING);

        if (m_crypto->ready()) m_crypto->reset();

        std::array<std::uint8_t, ThorQ::Crypto::PublicKeyLen> data;
        std::copy(fbsCrypto->data()->begin(), fbsCrypto->data()->end(), data.begin());

        if (m_crypto->generateKeyPair() && m_crypto->agreeAsClient(data))
        {
            m_crypto->getPublicKey(data);

            // Build flatbuffer
            flatbuffers::FlatBufferBuilder fbsBuilder;
            auto fbsVerify  = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Establish, fbsBuilder.CreateVector(data.data(), data.size())).Union();
            auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsVerify);
            fbsBuilder.Finish(fbsMessage);

            // Calculate packet size
            std::size_t size = ThorQ::calculatePacketSize(fbsBuilder.GetSize(), false);

            // Send it!
            ENetPacket* packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_RELIABLE);
            ThorQ::packetEncode(packet, fbsBuilder.GetBufferSpan());
            packetSend(packet, THORQ_CHANNEL::MAIN);
        }
        else
        {
            m_crypto->reset();
            SetCryptoState(THORQ_STATE_CRYPTO_NONE);
        }
        break;
    }
    case ThorQ::Serialization::Crypto::MessageType_Verify:
    {
        qDebug() << "CRYPTO: verify";
        SetCryptoState(THORQ_STATE_CRYPTO_VERIFYING);

        // Build flatbuffer
        flatbuffers::FlatBufferBuilder fbsBuilder;
        auto fbsVector  = fbsBuilder.CreateVector(fbsCrypto->data()->data(), fbsCrypto->data()->size());
        auto fbsVerify  = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Establish, fbsVector).Union();
        auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsVerify);
        fbsBuilder.Finish(fbsMessage);

        // Calculate packet size
        std::size_t size = ThorQ::calculatePacketSize(fbsBuilder.GetSize(), false);

        // Send it!
        ENetPacket* packet = enet_packet_create(nullptr, size, ENET_PACKET_FLAG_RELIABLE);
        ThorQ::packetEncode(packet, fbsBuilder.GetBufferSpan(), m_crypto);
        packetSend(packet, THORQ_CHANNEL::MAIN);
        break;
    }
    case ThorQ::Serialization::Crypto::MessageType_Acknowledge:
    {
        qDebug() << "CRYPTO: ack";
        SetCryptoState(THORQ_STATE_CRYPTO_ACTIVE);
        break;
    }
    default:
        qDebug() << "CRYPTO: Unexpected message:" << fbsCrypto->type();
        return;
    }
}

void Client::handleMessageSystemID(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsSystemId = reinterpret_cast<const ThorQ::Serialization::SystemId::Message*>(body);

    if (!fbsSystemId->Verify(fbsVerifier))
    {
        return;
    }

    /*
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
    }*/
}
void Client::handleMessageAccount(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::Account::Message*>(body);

    if (!fbsAccount->Verify(fbsVerifier))
    {
        return;
    }

    /*
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
    */
}

void Client::handleMessageFriendRequest(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::FriendRequest::Message*>(body);

    if (!fbsAccount->Verify(fbsVerifier))
    {
        return;
    }
}

void Client::handleMessageGroup(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsGroup = reinterpret_cast<const ThorQ::Serialization::Group::Message*>(body);

    if (!fbsGroup->Verify(fbsVerifier))
    {
        return;
    }
}

void Client::handleMessageModeration(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsModeration = reinterpret_cast<const ThorQ::Serialization::Moderation::Message*>(body);

    if (!fbsModeration->Verify(fbsVerifier))
    {
        return;
    }
}
void Client::handleMessageAnnouncement(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsAnnouncement = reinterpret_cast<const ThorQ::Serialization::Announcement::Message*>(body);

    if (!fbsAnnouncement->Verify(fbsVerifier))
    {
        return;
    }

    /*
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
    */
}
void Client::handleMessageCollar(const void* body, flatbuffers::Verifier fbsVerifier)
{
    auto fbsCollar = reinterpret_cast<const ThorQ::Serialization::Collar::Message*>(body);

    if (!fbsCollar->Verify(fbsVerifier))
    {
        return;
    }

    /*
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
    */
}

ENetPacket* Client::packetEncode(const std::span<std::uint8_t> data, bool encrypt, bool reliable)
{
    ENetPacket* packet = enet_packet_create(nullptr, ThorQ::calculatePacketSize(data.size(), encrypt), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED);

    if (packet != nullptr)
    {
        if (encrypt)
        {
            if (!ThorQ::packetEncode(packet, data, m_crypto))
            {
                qWarning() << "Failed to encode packet!";
                enet_packet_destroy(packet);
                packet = nullptr;
            }
        }
        else
        {
            if (!ThorQ::packetEncode(packet, data))
            {
                qWarning() << "Failed to encode packet!";
                enet_packet_destroy(packet);
                packet = nullptr;
            }
        }
    }
    else
    {
        qWarning() << "Failed to allocate packet!";
    }

    return packet;
}

bool Client::packetDecode(const ENetPacket* packet, std::vector<uint8_t>& payload)
{
    return ThorQ::packetDecode(packet, payload, m_crypto);
}

bool Client::packetSend(ENetPacket* packet, THORQ_CHANNEL ch)
{
    if (packet != nullptr)
    {
        return enet_peer_send(m_peer, (std::uint8_t)ch, packet) == 0;
    }

    return false;
}

void Client::requestEncryptionHandshake()
{
    qDebug() << "CRYPTO send";
    if (ConnectionState() != THORQ_STATE_CONNECTION_CONNECTED)
        return;

    SetCryptoState(THORQ_STATE_CRYPTO_REQUESTED);

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsRequest = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, ThorQ::Serialization::Crypto::MessageType_Request);
    auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsRequest.Union());
    fbsBuilder.Finish(fbsMessage);

    packetSend(packetEncode(fbsBuilder.GetBufferSpan(), false, true), THORQ_CHANNEL::MAIN);
    qDebug() << "done";
}

void Client::handleDisconnect(THORQ_DISCONNECT_REASON reason)
{
	m_peer = nullptr;
	SetConnectionState(THORQ_STATE_CONNECTION_DISCONNECTED);

    switch (reason)
    {
    case THORQ_DISCONNECT_REASON::TIMED_OUT:
        emit Warning(tr("Connection timed out"));
        return;
    case THORQ_DISCONNECT_REASON::VERSION_INCOMPATIBLE:
        emit Error(tr("Please update you application\nContact the Dev:\nYameroDev#9058"));
        break;
    case THORQ_DISCONNECT_REASON::CRYPTO_FAILED:
        emit Error(tr("Encryption failed"));
        break;
    case THORQ_DISCONNECT_REASON::SHUTDOWN_CLOSED:
        emit Warning(tr("Server shut down"));
        break;
    case THORQ_DISCONNECT_REASON::SHUTDOWN_MAINTANENCE:
        emit Warning(tr("Server is undergoing maintenance"));
        break;
    case THORQ_DISCONNECT_REASON::KICKED:
        emit Error(tr("You have been kicked"));
        break;
    case THORQ_DISCONNECT_REASON::BANNED:
        emit Error(tr("You have been banned"));
        break;
    default:
        emit Warning(tr("Disconnected for unknown reason"));
        break;
    }
}
