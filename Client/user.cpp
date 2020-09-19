#include "user.h"

#include "client.h"

ThorQ::User::User(Client* client)
    : QObject(client)
    , m_name()
{

}

const QString &ThorQ::User::name() const
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

void ThorQ::User::sessionRequest()
{

}
void ThorQ::User::sessionAccept()
{

}
void ThorQ::User::sessionDeny()
{

}

void ThorQ::User::setName(const QString& name)
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
