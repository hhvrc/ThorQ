#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <enums.h>
#include <atomic>
#include <mutex>
#include <vector>

#include <thorq_payload.h>
#include <thorq_payload_version.h>

// Forward declerations
class QTimer;
class QThread;
class QElapsedTimer;
namespace ThorQ { class Crypto; }
typedef struct _ENetPeer    ENetPeer;
typedef struct _ENetHost    ENetHost;
typedef struct _ENetPacket  ENetPacket;
typedef struct _ENetAddress ENetAddress;

/** @class The Client class
 */
class Client : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Client)

    /**
     * @param client
     */
	Client(ENetHost* client);
public:
    /**
     * @return
     */
	static Client* NewClient();

    /**
     */
	~Client();

    /**
     * @return
     */
	static QString Version();

    /**
     * @return
     */
	uint Ping() const;

    /**
     * @return
     */
	thorq_connection_state_t ConnectionState() const;

    /**
     * @return
     */
	thorq_crypto_state_t CryptoState() const;

    /**
     * @return
     */
	thorq_auth_state_t AuthState() const;

    /**
     * @return
     */
	thorq_login_state_t LoginState() const;

    /**
     * @return
     */
	thorq_session_state_t SessionState() const;
public slots:
    /**
     * @param address
     * @param port
     */
	void Connect(const char* address, std::uint16_t port);

    /**
     * @brief blah blah blah
     */
	void Reconnect();

    /**
     * @brief blah blah blah
     */
	void Disconnect();

    /**
     * @param Username
     */
	void Login(const QString& Username);

    /**
     * @brief blah blah blah
     */
	void Logout();

    /**
     * @param username
     */
	void RequestSession(const QString& username);

    /**
     * @brief blah blah blah
     */
	void AcceptRequest();

    /**
     * @brief blah blah blah
     */
	void DenyRequest();

    /**
     * @brief LeaveSession
     */
	void LeaveSession();

    /**
     * @param strengt
     */
	void SetShock(std::uint8_t strengt);

    /**
     * @param strength
     */
	void SetVibrate(std::uint8_t strength);

    /**
     * @param strength
     */
	void SetBeep(std::uint8_t strength);

    /**
     * @param sensitivity
     */
	void EnableAuto(std::uint8_t sensitivity);

    /**
     */
	void DisableAuto();

    /**
     */
	void SendImpulse();

    /**
     * @param regKey
     */
	void SetRegistrationKey(const QString& regKey);
signals:
    /**
     * @param Address
     */
	void AddressChanged(const QString& Address);

    /**
     * @param Port
     */
	void PortChanged(std::uint16_t Port);

    /**
     * @param ping
     */
	void PingChanged(uint ping);

    /**
     * @param state
     */
	void ConnectionStateChanged(thorq_connection_state_t state);

    /**
     * @param state
     */
	void CryptoStateChanged(thorq_crypto_state_t state);

    /**
     * @param state
     */
	void AuthStateChanged(thorq_auth_state_t state);

    /**
     * @param state
     */
	void LoginStateChanged(thorq_login_state_t state);

    /**
     * @param state
     */
	void SessionStateChanged(thorq_session_state_t state);

    /**
     * @param username
     */
	void usernameChanged(const QString& username);

    /**
     * @param username
     */
	void partnerChanged(const QString& username);

    /**
     * @param user
     * @param state
     */
	void userUpdate(const QString& user, std::uint8_t state);

    /**
     * @param user
     */
	void UserOffline(const QString& user);

    /**
     * @param strength
     */
	void ReceivedShock(std::uint8_t strength);

    /**
     * @param strength
     */
	void ReceivedVibrate(std::uint8_t strength);

    /**
     * @param count
     */
	void ReceivedBeep(std::uint8_t count);

    /**
     * @param sensitivity
     * @param shockStrength
     * @param vibrateStrength
     * @param beepCount
     */
	void ReceivedAuto(std::uint8_t sensitivity, std::uint8_t shockStrength, std::uint8_t vibrateStrength, std::uint8_t beepCount);

    /**
     * @brief blah blah blah
     */
	void ReceivedManual();

    /**
     * @brief blah blah blah
     */
	void RequestingRegistrationKey();

    /**
     * @brief Error
     * @param what
     */
	void Error(const QString& what);
private slots:
    /**
     * @brief blah blah blah
     */
	void Service();

	void SetPing(std::uint16_t ping);

    /**
     * @brief [Thread-Safe] Sets the state of the connection
     * @brief Will affect CryptoState, as there can be no cryptographic agreement if client is disconnected
	 *
     * @param state State to set
	 */
	void SetConnectionState(thorq_connection_state_t state);

    /**
     * @brief [Thread-Safe] Sets the state of the cryptographic agreement
     * @brief Will affect AuthState, as a client should not be sending sensitive data over a unsecured connection
     *
     * @param state State to set
	 */
	void SetCryptoState(thorq_crypto_state_t state);

    /**
     * @brief [Thread-Safe] Sets the state of the authentication
     * @brief Will affect LoginState, as a client should not be able to log in without having bought the application
     *
     * @param state State to set
	 */
	void SetAuthState(thorq_auth_state_t state);

    /**
     * @brief [Thread-Safe] Sets the state of the login
     * @brief Will affect SessionState, as a user cannot be in a session without being logged in
     *
     * @param state State to set
	 */
	void SetLoginState(thorq_login_state_t state);

    /**
     * @brief [Thread-Safe] Sets the state of the session
     *
     * @param state State to set
	 */
	void SetSessionState(thorq_session_state_t state);

    /**
     * @brief SetUsername
     * @param username
     */
	void SetUsername(const QString& username);

    /**
     * @brief SetPartner
     * @param username
     */
	void SetPartner(const QString& username);

    /**
     * @brief HandleMessage
     * @param packet
     */
	void HandleMessage(ENetPacket* packet);

    /**
     * @brief handleMessageVersion
     * @param payload
     */
	void handleMessageVersion(const thorq_payload_t& payload);

    /**
     * @brief handleMessageHeartbeat
     * @param payload
     */
    void handleMessageHeartbeat();

    /**
     * @brief handleMessageCrypto
     * @param payload
     */
	void handleMessageCrypto(const thorq_payload_t& payload);

    /**
     * @brief handleMessageAuth
     * @param payload
     */
	void handleMessageAuth(const thorq_payload_t& payload);

    /**
     * @brief SendPayload
     * @param payload
     * @param encrypt
     * @param reliable
     */
	void SendPayload(const thorq_payload_t& payload, bool encrypt = true, bool reliable = true);

    /**
     * @brief requestEncryptionHandshake
     */
	void requestEncryptionHandshake();

	void handleDisconnect(std::uint32_t reason);
private:
	ThorQ::Crypto* m_crypto;

	std::atomic<thorq_connection_state_t> m_connectionState;
	std::atomic<thorq_crypto_state_t> m_cryptoState;
	std::atomic<thorq_auth_state_t> m_authState;
	std::atomic<thorq_login_state_t> m_loginState;
	std::atomic<thorq_session_state_t> m_sessionState;
	std::atomic_uint m_ping;

	std::mutex l_username;
	std::string m_username;

	std::mutex l_partnerName;
	std::string m_partnerName;

	std::mutex l_requestedPartner;
	std::string m_requestedPartner;

	std::mutex l_requestingPartner;
	std::string m_requestingPartner;

	std::mutex l_registrationKey;
	std::string m_registrationKey;

	std::atomic<std::uint16_t> m_actionFlags;
    std::atomic<std::uint64_t> m_collarState;

	QThread* m_thread;
	QTimer* m_serviceTimer;

	bool m_awaitingPing;
    std::uint64_t m_lastPing;
	QElapsedTimer* m_pingTimer;

	ENetHost* m_host;
	ENetPeer* m_peer;

	std::mutex    l_requestedHost;
	std::string   m_requestedHostName;
	std::uint16_t m_requestedHostPort;

	ENetAddress* m_address;
};

#endif // CLIENT_H
