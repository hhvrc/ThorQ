#ifndef SINGETONS_H
#define SINGETONS_H

#include <QSet>

#include "typedefs_server.h"

#ifdef SINGLETON_BASE
ThorQ::Server g_server;
QSet<ThorQ::Account*> g_accounts;
#else
extern ThorQ::Server g_server;
extern QSet<ThorQ::Account*> g_accounts;
#endif

#endif // SINGETONS_H
