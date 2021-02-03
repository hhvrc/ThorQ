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

    QUuid id() const;

    enum class Presence : std::uint8_t {
        Offline,
        Idle,
        Online,
    };
    Q_ENUM(Presence)
    Presence presence() const;

    QString username() const;

    bool isInSession() const;
    bool isInSteamVR() const;
    bool hasCollarConnected() const;
    bool isRequestingSession() const;
signals:
    void usernameChanged(QString name);

    void sessionRequested();
    void sessionStarted();
    void sessionStopped();

    void inSessionChanged(bool isInSession);
    void inSteamVRChanged(bool isInSteamVR);
    void hasCollarConnectedChanged(bool hasCollarConnected);
    void isRequestingSessionChanged(bool isRequestingSession);
public slots:
    void setName(QString name);

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
    QString m_name;
    bool m_isInSession;
    bool m_isInSteamVR;
    bool m_hasCollarConnected;
    bool m_isRequestingSession;
};
}

#endif // USER_H
