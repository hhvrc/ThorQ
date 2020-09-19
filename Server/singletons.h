#ifndef SINGETONS_H
#define SINGETONS_H

#include <QSet>

#include <typedefs.h>

#ifdef SINGLETON_BASE
ENetHost* g_server = nullptr;
QSet<ThorQ::Account*> g_accounts;
QSet<ThorQ::Instance*> g_onlineInstances;
#else
extern ENetHost* g_server;
extern QSet<ThorQ::Account*> g_accounts;
extern QSet<ThorQ::Instance*> g_onlineInstances;
#endif

#endif // SINGETONS_H
