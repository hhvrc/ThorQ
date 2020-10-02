#ifndef SINGETONS_H
#define SINGETONS_H

#include <QSet>

#include "typedefs_server.h"

#ifdef SINGLETON_BASE
ThorQ::Server g_server;
QSet<ThorQ::Account*> g_accounts;
QSet<ThorQ::Instance*> g_sessions;
#else
extern ThorQ::Server g_server;
extern QSet<ThorQ::Account*> g_accounts;
extern QSet<ThorQ::Instance*> g_sessions;
#endif

#endif // SINGETONS_H
