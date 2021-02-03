#include "user.h"

#include <QObject>

ThorQ::User::User(QUuid userID, QObject* parent)
    : QObject(parent)
    , m_id(userID)
    , m_name()
{

}

QUuid ThorQ::User::id() const
{
    return m_id;
}

QString ThorQ::User::username() const
{
    return m_name;
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

void ThorQ::User::setName(QString name)
{
    if (m_name != name)
    {
        m_name = name;
        emit usernameChanged(name);
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
