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
    int ClientState() const;
    int CryptoState() const;
    int SessionState() const;

    QString Username() const;
    UserData Partner() const;
    QList<UserData> OnlineUsers() const;
public slots:
    void Connect(const char* address, int port);
    void Disconnect();

    void Login(const QString& Username);
    void Logout();

    void RequestSession(const QString& username);
    void AcceptRequest();
    void DenyRequest();
    void LeaveSession();

    void SetShock(bool enable, int strength);
    void SetVibrate(bool enable, int strength);
    void SetBeep(bool enable, int strength);
    void SetAuto(bool enable, int sensitivity, int shockStrength, int vibrateStrength, int beepCount);
signals:
    void AddressChanged(const QString& Address);
    void PortChanged(int Port);

    void PingChanged(int ping);
    void ClientStateChanged(int state);
    void CryptoStateChanged(int state);
    void SessionStateChanged(int state);

    void usernameChanged(const QString& username);
    void PartnerChanged(const UserData& user);

    void UserOnline(const UserData& user);
    void UserOffline(const QString& user);
    void UserCollarOn(const QString& user);
    void UserCollarOff(const QString& user);
    void UserInSession(const QString& user);
    void UserLeftSession(const QString& user);

    void ReceivedShock(int strength);
    void ReceivedVibrate(int strength);
    void ReceivedBeep(int count);
    void ReceivedAuto(int sensitivity, int shockStrength, int vibrateStrength, int beepCount);
    void ReceivedManual();

    void Error(const QString& what);
private slots:
    void Run();

    void SetPing(int ping);
    void SetClientState(int state);
    void SetCryptoState(int state);
    void SetSessionState(int state);

    void SetUsername(const QString& username);
    void SetPartner(const QString& username);

    void UpsertOnlineUser(const UserData& user);
    void RemoveOnlineUser(const QString& user);

    void HandleMessage(ENetPacket* packet);

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
    std::uint8_t GetFlag(bool withHeartbeat = false);
private:
	ThorQ::Crypto* m_crypto;

    std::atomic_int m_clientState;
    std::atomic_int m_cryptoState;
    std::atomic_int m_sessionState;
    std::atomic_int m_ping;

    std::mutex l_username;
    std::string m_username;

    std::mutex l_requestedPartner;
    std::string m_requestedPartner;

    std::mutex l_requestingPartner;
    std::string m_requestingPartner;

    std::mutex l_onlineUsers;
    QString m_partnerName;
    QList<UserData> m_onlineUsers;

    std::atomic_uint8_t m_actionFlags;
    std::atomic_uint8_t m_collarFlags;
    std::atomic_int m_shockValue;
    std::atomic_int m_vibrateValue;
    std::atomic_int m_beepValue;
    std::atomic_int m_autoSensitivity;
    std::atomic_int m_autoShock;
    std::atomic_int m_autoVibrate;
    std::atomic_int m_autoBeep;

	QThread* m_thread;

    bool m_awaitingPing;
    QElapsedTimer* m_pingTimer;

    ENetHost* m_host;
    ENetPeer* m_peer;

    std::mutex l_address;
    ENetAddress* m_address;
};

#endif // CLIENT_H
