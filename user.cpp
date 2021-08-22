#include "user.h"

#include <QObject>

ThorQ::User::User(QUuid userID, QObject* parent)
    : QObject(parent)
    , m_id(userID)
    , m_username()
    , m_nickname()
    , m_presence(Presence::Offline)
    , m_isInSession(false)
    , m_isInSteamVR(false)
    , m_hasCollarConnected(false)
    , m_isRequestingSession(false)
{
}

QString ThorQ::User::username() const
{
    return m_username;
}

bool ThorQ::User::isInSession() const
{
    return m_isInSession;
}

bool ThorQ::User::isInSteamVR() const
{
    return m_isInSteamVR;
}

bool ThorQ::User::hasCollarConnected() const
{
    return m_hasCollarConnected;
}

bool ThorQ::User::isRequestingSession() const
{
    return m_isRequestingSession;
}

void ThorQ::User::setUsername(const QString& username)
{
    if (m_username != username)
    {
        m_username = username;
        emit usernameChanged(username);
    }
}

void ThorQ::User::setNickname(const QString& nickname)
{
    if (m_username != nickname)
    {
        m_username = nickname;
        emit usernameChanged(nickname);
    }
}

void ThorQ::User::sessionRequest()
{

}
void ThorQ::User::sessionAccept()
{

}
void ThorQ::User::sessionDeny()
{

}

void ThorQ::User::setIsInSession(bool isInSession)
{

}
void ThorQ::User::setIsInSteamVR(bool isInSteamVR)
{

}
void ThorQ::User::setHasCollarConnected(bool hasCollarConnected)
{

}
void ThorQ::User::setIsRequestingSession(bool isRequestingSession)
{

}
