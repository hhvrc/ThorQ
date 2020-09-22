#ifndef SINGETONS_H
#define SINGETONS_H

#include <QSet>

#include <typedefs.h>

#ifdef SINGLETON_BASE
ENetHost* g_server = nullptr;
QSet<ENetPeer*> g_peers;
QSet<ThorQ::Account*> g_accounts;
QSet<ThorQ::Session*> g_sessions;
#else
extern ENetHost* g_server;
extern QSet<ENetPeer*> g_peers;
extern QSet<ThorQ::Account*> g_accounts;
extern QSet<ThorQ::Session*> g_sessions;
#endif

#endif // SINGETONS_H
