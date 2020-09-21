#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <enums.h>
#include <atomic>
#include <mutex>
#include <vector>

#include <typedefs.h>
#include <thorq_payload_version.h>

#include "user.h"

/// @class Client
class Client : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Client)

    friend ThorQ::User;

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
    quint16 Rtt() const;

    /**
     * @return
     */
	THORQ_STATE_CONNECTION ConnectionState() const;

    /**
     * @return
     */
	THORQ_STATE_CRYPTO CryptoState() const;

    /**
     * @return
     */
	THORQ_STATE_AUTH AuthState() const;

    /**
     * @return
     */
	THORQ_STATE_LOGIN LoginState() const;

    /**
     * @return
     */
	THORQ_STATE_SESSION SessionState() const;
public slots:
    /**
     * @param address
     * @param port
     */
    void Connect(const char* address, quint16 port);

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
    void SetShock(quint8 strengt);

    /**
     * @param strength
     */
    void SetVibrate(quint8 strength);

    /**
     * @param strength
     */
    void SetBeep(quint8 strength);

    /**
     * @param sensitivity
     */
    void EnableAuto(quint8 sensitivity);

    /**
     */
	void DisableAuto();

    /**
     */
	void SendImpulse();

    /**
     * @param regKey
     */
    void submitRegistrationKey(const QString& regKey);
signals:
    /**
     * @param Address
     */
	void AddressChanged(const QString& Address);

    /**
     * @param Port
     */
    void PortChanged(quint16 Port);

    /**
     * @param ping
     */
    void RttChanged(quint16 ping);

    /**
     * @param state
     */
	void ConnectionStateChanged(THORQ_STATE_CONNECTION state);

    /**
     * @param state
     */
	void CryptoStateChanged(THORQ_STATE_CRYPTO state);

    /**
     * @param state
     */
	void AuthStateChanged(THORQ_STATE_AUTH state);

    /**
     * @param state
     */
	void LoginStateChanged(THORQ_STATE_LOGIN state);

    /**
     * @param state
     */
	void SessionStateChanged(THORQ_STATE_SESSION state);

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
    void userUpdate(const QString& user, quint8 state);

    /**
     * @param user
     */
	void UserOffline(const QString& user);

    /**
     * @param user
     */
    void SessionRequested(const QString& user);

    /**
     * @param user
     */
    void SessionStarted(const QString& user);

    /**
     * @param strength
     */
    void ReceivedShock(quint8 strength);

    /**
     * @param strength
     */
    void ReceivedVibrate(quint8 strength);

    /**
     * @param count
     */
    void ReceivedBeep(quint8 count);

    /**
     * @param sensitivity
     * @param shockStrength
     * @param vibrateStrength
     * @param beepCount
     */
    void ReceivedAuto(quint8 sensitivity, quint8 shockStrength, quint8 vibrateStrength, quint8 beepCount);

    /**
     * @brief blah blah blah
     */
	void ReceivedManual();

    /**
     * @brief blah blah blah
     */
	void RequestingRegistrationKey();

	void Warning(const QString& what);
	void Error(const QString& what);
    void Announcement(const QString& what);
private slots:
    /**
     * @brief blah blah blah
     */
	void Service();

    void SetRtt(quint16 rtt);

    /**
     * @brief [Thread-Safe] Sets the state of the connection
     * @brief Will affect CryptoState, as there can be no cryptographic agreement if client is disconnected
	 *
     * @param state State to set
	 */
	void SetConnectionState(THORQ_STATE_CONNECTION state);

    /**
     * @brief [Thread-Safe] Sets the state of the cryptographic agreement
     * @brief Will affect AuthState, as a client should not be sending sensitive data over a unsecured connection
     *
     * @param state State to set
	 */
	void SetCryptoState(THORQ_STATE_CRYPTO state);

    /**
     * @brief [Thread-Safe] Sets the state of the authentication
     * @brief Will affect LoginState, as a client should not be able to log in without having bought the application
     *
     * @param state State to set
	 */
	void SetAuthState(THORQ_STATE_AUTH state);

    /**
     * @brief [Thread-Safe] Sets the state of the login
     * @brief Will affect SessionState, as a user cannot be in a session without being logged in
     *
     * @param state State to set
	 */
	void SetLoginState(THORQ_STATE_LOGIN state);

    /**
     * @brief [Thread-Safe] Sets the state of the session
     *
     * @param state State to set
	 */
	void SetSessionState(THORQ_STATE_SESSION state);

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

    /// These should be self-explanatory
    void HandleMessage(ENetPacket* packet);
    void handleMessageVersion(std::vector<std::uint8_t>& payload);
    void handleMessageCrypto(std::vector<std::uint8_t>& payload);
    void handleMessageAuth(std::vector<std::uint8_t>& payload);
    void handleMessageAnnouncement(std::vector<std::uint8_t>& payload);
    void handleMessageHeartbeat(std::vector<std::uint8_t>& payload);
    void handleMessageEvent(std::vector<std::uint8_t>& payload);
    void handleMessageCommand(std::vector<std::uint8_t>& payload);
    void handleMessageCommandAck(std::vector<std::uint8_t>& payload);
    void handleMessageNotification(std::vector<std::uint8_t>& payload);
    void handleMessageCollar(std::vector<std::uint8_t>& payload);

    /**
     * @brief SendPayload
     * @param payload
     * @param encrypt
     * @param reliable
     */
    void SendPayload(std::vector<std::uint8_t>& payload, bool encrypt = true, bool reliable = true);

    /**
     * @brief requestEncryptionHandshake
     */
	void requestEncryptionHandshake();

    void handleDisconnect(quint32 reason);
private:
	ThorQ::Crypto* m_crypto;

	std::atomic<THORQ_STATE_CONNECTION> m_connectionState;
	std::atomic<THORQ_STATE_CRYPTO> m_cryptoState;
	std::atomic<THORQ_STATE_AUTH> m_authState;
	std::atomic<THORQ_STATE_LOGIN> m_loginState;
	std::atomic<THORQ_STATE_SESSION> m_sessionState;
    std::atomic<quint16> m_rtt;

    std::mutex l_loginInfo;
	QString m_username;
	QString m_password;

	std::mutex l_partnerName;
	QString m_partnerName;

	std::mutex l_requestedPartner;
	QString m_requestedPartner;

	std::mutex l_requestingPartner;
	QString m_requestingPartner;

	std::mutex l_registrationKey;
	QString m_registrationKey;

	std::atomic<quint16> m_actionFlags;
	std::atomic<quint64> m_collarState;

	QThread* m_thread;
	QTimer* m_serviceTimer;

    bool m_awaitingHeartbeat;
    std::uint64_t m_lastCheck;
    std::uint16_t m_heartbeatInterval;
    QElapsedTimer* m_heartbeatTimer;

	ENetHost* m_host;
	ENetPeer* m_peer;

	std::mutex    l_requestedHost;
	QString m_requestedHostName;
	quint16 m_requestedHostPort;

	ENetAddress* m_address;
};

#endif // CLIENT_H
