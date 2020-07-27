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
class QThread;
class QElapsedTimer;
namespace ThorQ { class Crypto; }
typedef struct _ENetPeer    ENetPeer;
typedef struct _ENetHost    ENetHost;
typedef struct _ENetPacket  ENetPacket;
typedef struct _ENetAddress ENetAddress;

#include "userdata.h"

class Client : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Client)
	Client(ENetHost* client);
public:
	static Client* NewClient();
	~Client();

	static QString Version();

	int Ping() const;
	thorq_connection_state_t ConnectionState() const;
	thorq_crypto_state_t CryptoState() const;
	thorq_auth_state_t AuthState() const;
	thorq_login_state_t LoginState() const;
	thorq_session_state_t SessionState() const;
public slots:
	void Connect(const char* address, int port);
	void Reconnect();
	void Disconnect();

	void Login(const QString& Username);
	void Logout();

	void RequestSession(const QString& username);
	void AcceptRequest();
	void DenyRequest();
	void LeaveSession();

	void SetShock(bool enable, int strength = -1);
	void SetVibrate(bool enable, int strength = -1);
	void SetBeep(bool enable, int strength = -1);
	void SetAuto(bool enable, int sensitivity, int shockStrength, int vibrateStrength, int beepCount);

	void SetRegistrationKey(const QString& regKey);
signals:
	void AddressChanged(const QString& Address);
	void PortChanged(int Port);

	void PingChanged(int ping);
	void ConnectionStateChanged(thorq_connection_state_t state);
	void CryptoStateChanged(thorq_crypto_state_t state);
	void AuthStateChanged(thorq_auth_state_t state);
	void LoginStateChanged(thorq_login_state_t state);
	void SessionStateChanged(thorq_session_state_t state);

	void usernameChanged(const QString& username);
	void partnerChanged(const UserData& user);

	void userUpdate(const UserData& user);
	void UserOffline(const QString& user);

	void ReceivedShock(int strength);
	void ReceivedVibrate(int strength);
	void ReceivedBeep(int count);
	void ReceivedAuto(int sensitivity, int shockStrength, int vibrateStrength, int beepCount);
	void ReceivedManual();

	void RequestingRegistrationKey();

	void Error(const QString& what);
private slots:
	void Run();

	void SetPing(int ping);

	/**
	 * @brief SetConnectionState
	 *
	 * Thread-safe
	 *
	 * Sets the state of the connection
	 * Will affect CryptoState, as there can be no cryptographic agreement if client is disconnected
	 *
	 * @param newState
	 */
	void SetConnectionState(thorq_connection_state_t state);

	/**
	 * @brief SetCryptoState
	 *
	 * Thread-safe
	 *
	 * Sets the state of the cryptographic agreement
	 * Will affect AuthState, as a client should not be sending sensitive data over a unsecured connection
	 *
	 * @param newState
	 */
	void SetCryptoState(thorq_crypto_state_t state);

	/**
	 * @brief SetAuthState
	 *
	 * Thread-safe
	 *
	 * Sets the state of the authentication
	 * Will affect LoginState, as a client should not be able to log in without having bought the application
	 *
	 * @param newState
	 */
	void SetAuthState(thorq_auth_state_t state);

	/**
	 * @brief SetLoginState
	 *
	 * Thread-safe
	 *
	 * Sets the state of the login
	 * Will affect SessionState, as a user cannot be in a session without being logged in
	 *
	 * @param newState
	 */
	void SetLoginState(thorq_login_state_t state);

	/**
	 * @brief SetSessionState
	 *
	 * Thread-safe
	 *
	 * Sets the state of the session
	 *
	 * @param newState
	 */
	void SetSessionState(thorq_session_state_t state);

	void SetUsername(const QString& username);
	void SetPartner(const QString& username);

	void HandleMessage(ENetPacket* packet);
	void HandleMessageVersion(const thorq_payload_t& payload);
	void HandleMessageHeartbeat(const thorq_payload_t& payload);
	void HandleMessageCrypto(const thorq_payload_t& payload);
	void HandleMessageAuth(const thorq_payload_t& payload);

	void SendPayload(const thorq_payload_t& payload, bool encrypt = true, bool reliable = true);

	void requestEncryptionHandshake();
private:
	ThorQ::Crypto* m_crypto;

	std::atomic<thorq_connection_state_t> m_connectionState;
	std::atomic<thorq_crypto_state_t> m_cryptoState;
	std::atomic<thorq_auth_state_t> m_authState;
	std::atomic<thorq_login_state_t> m_loginState;
	std::atomic<thorq_session_state_t> m_sessionState;
	std::atomic_int m_ping;

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

	std::atomic_uint m_actionFlags;
	std::atomic_uint m_collarFlags;
	std::atomic_uint m_shockValue;
	std::atomic_uint m_vibrateValue;
	std::atomic_uint m_beepValue;
	std::atomic_uint m_autoSensitivity;
	std::atomic_uint m_autoShock;
	std::atomic_uint m_autoVibrate;
	std::atomic_uint m_autoBeep;

	QThread* m_thread;

	bool m_awaitingPing;
	QElapsedTimer* m_pingTimer;

	ENetHost* m_host;
	ENetPeer* m_peer;

	std::mutex l_requestedHost;
	std::string m_requestedHostName;
	int         m_requestedHostPort;

	ENetAddress* m_address;
};

#endif // CLIENT_H
