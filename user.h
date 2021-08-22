#ifndef USER_H
#define USER_H

#include <QObject>
#include <QUuid>
#include <QString>

namespace ThorQ {
class User : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(User)
public:
    User(QUuid userID, QObject* parent = nullptr);

    QUuid id() const { return m_id; }

    QString username() const;

    QString nickname() const;

    enum class Presence : std::uint8_t {
        Offline,
        Idle,
        Online,
    };
    Q_ENUM(Presence)
    Presence presence() const;

    bool isInSession() const;
    bool isInSteamVR() const;
    bool hasCollarConnected() const;
    bool isRequestingSession() const;
signals:
    void usernameChanged(const QString&);
    void nicknameChanged(const QString&);

    void sessionRequested();
    void sessionStarted();
    void sessionStopped();

    void inSessionChanged(bool);
    void inSteamVRChanged(bool);
    void hasCollarConnectedChanged(bool);
    void isRequestingSessionChanged(bool);
public slots:
    void setUsername(const QString& username);
    void setNickname(const QString& nickname);

    void sessionRequest();
    void sessionAccept();
    void sessionDeny();

protected slots:
    void setIsInSession(bool isInSession);
    void setIsInSteamVR(bool isInSteamVR);
    void setHasCollarConnected(bool hasCollarConnected);
    void setIsRequestingSession(bool isRequestingSession);
private:
    const QUuid m_id;
    QString m_username;
    QString m_nickname;
    Presence m_presence;
    bool m_isInSession;
    bool m_isInSteamVR;
    bool m_hasCollarConnected;
    bool m_isRequestingSession;
};
}

#endif // USER_H
