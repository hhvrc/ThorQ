#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>

// Forward declerations
class QThread;
class QElapsedTimer;
namespace ThorQ { class Crypto; }
typedef struct _ENetPeer   ENetPeer;
typedef struct _ENetHost   ENetHost;
typedef struct _ENetPacket ENetPacket;

class Client : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Client)
	Client();

	Q_PROPERTY(QString address		READ address		WRITE setAddress	NOTIFY addressChanged)
	Q_PROPERTY(int port				READ port			WRITE setPort		NOTIFY portChanged)
	Q_PROPERTY(bool connected		READ connected							NOTIFY connectedChanged)
	Q_PROPERTY(bool authenticated	READ authenticated						NOTIFY authenticatedChanged)
	Q_PROPERTY(QString username		READ username							NOTIFY usernameChanged)
public:
	static Client* NewClient(QString hostname, int port);
	~Client();

	static QString Version();

	QString address() const;
	int port() const;
	bool connected() const;
	bool authenticated() const;
	QString username() const;

	void setAddress(const QString& address);
	void setPort(int port);
signals:
	void portChanged(int port);
	void addressChanged(const QString& address);

	void connectedChanged(bool connected);
	void authenticatedChanged(bool connected);

	void Connected();
	void Disconnected();
	void TimedOut();
	void Error(const QString& what);

	void dataRx();
	void dataTx();

	void LoggedIn();
	void LoginFailed();
	void usernameChanged(const QString& username);

	void ReceivedA(int i);
	void ReceivedB(int i);
	void ReceivedC(int i);
public slots:
	void Connect();
	void Disconnect();

	void Login(const QString& username);
	void Logout();

	void SetAuto(int a, int b, int c);
	void SendA(int i);
	void SendB(int i);
	void SendC(int i);
private slots:
	void run();
private:
	void setConnected(bool connected);

	void handleConnect();
	void handleMessage(ENetPacket* packet);

	void SendRaw(const std::vector<std::uint8_t>& data, bool unreliable = false);
	void SendRaw(const std::uint8_t* data, std::size_t len, bool unreliable = false);
	void SendEncrypted(const std::vector<std::uint8_t>& data, bool unreliable = false);
	void SendEncrypted(const std::uint8_t* data, std::size_t len, bool unreliable = false);
	void SendEncrypted(std::uint32_t meta);
	void SendEncrypted(std::uint32_t meta, const std::string& message);

	ThorQ::Crypto* m_crypto;

	QString m_address;
	int m_port;
	bool m_connected;
	bool m_reconnect;
	QString m_username;
	QString m_requestedUsername;

	QThread* m_thread;
	QElapsedTimer* m_pingTimer;

	ENetPeer* m_peer;
	ENetHost* m_client;
};

#endif // CLIENT_H
