#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>

class Client;

namespace ThorQ {
class User : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(User)
protected:
    User() = delete;
    User(QObject*) = delete;
    User(Client* client);
public:
    const QString& name() const;

    bool isInSession() const;
    bool isInSteamVR() const;
    bool hasCollarConnected() const;
    bool isRequestingSession() const;
signals:
    void sessionRequested();
    void sessionStarted();
    void sessionStopped();

    void nameChanged(const QString& name);
    void inSessionChanged(bool isInSession);
    void inSteamVRChanged(bool isInSteamVR);
    void hasCollarConnectedChanged(bool hasCollarConnected);
    void isRequestingSessionChanged(bool isRequestingSession);

public slots:
    void sessionRequest();
    void sessionAccept();
    void sessionDeny();

protected slots:
    void setName(const QString& name);
    void setIsInSession(bool isInSession);
    void setIsInSteamVR(bool isInSteamVR);
    void setHasCollarConnected(bool hasCollarConnected);
    void setIsRequestingSession(bool isRequestingSession);

    friend Client;
private:
    QString m_name;
    bool m_isInSession;
    bool m_isInSteamVR;
    bool m_hasCollarConnected;
    bool m_isRequestingSession;
};
}

#endif // USER_H
