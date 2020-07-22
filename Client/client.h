#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <enums.h>
#include <atomic>
#include <mutex>

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
	thorq_login_state_t LoginState() const;
	thorq_session_state_t SessionState() const;
public slots:
    bool Connect(const char* address, int port);
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
signals:
    void AddressChanged(const QString& Address);
    void PortChanged(int Port);

    void PingChanged(int ping);
	void ConnectionStateChanged(thorq_connection_state_t state);
	void CryptoStateChanged(thorq_crypto_state_t state);
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
	 * Will affect LoginState, as a client should not be logged in on a unsecured connection
	 *
	 * @param newState
	 */
	void SetCryptoState(thorq_crypto_state_t state);

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

    void requestEncryptionHandshake();
    void SendHeartbeat();

    void SendRaw(std::uint32_t meta, bool unreliable = false);
    void SendRaw(std::uint32_t meta, const std::string& message, bool unreliable = false);
    void SendRaw(const std::vector<std::uint8_t>& data, bool unreliable = false);
    void SendRaw(const std::uint8_t* data, std::size_t len, bool unreliable = false);
    void SendEncrypted(std::uint32_t meta, bool unreliable = false);
    void SendEncrypted(std::uint32_t meta, const std::string& message, bool unreliable = false);
    void SendEncrypted(const std::vector<std::uint8_t>& data, bool unreliable = false);
    void SendEncrypted(const std::uint8_t* data, std::size_t len, bool unreliable = false);
private:
    std::uint8_t GetFlag();
private:
	ThorQ::Crypto* m_crypto;

	std::atomic<thorq_connection_state_t> m_connectionState;
	std::atomic<thorq_crypto_state_t> m_cryptoState;
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

    std::mutex l_address;
    ENetAddress* m_address;
};

#endif // CLIENT_H
