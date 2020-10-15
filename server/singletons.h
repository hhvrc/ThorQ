#ifndef SINGETONS_H
#define SINGETONS_H

#include <set>

#include "typedefs_server.h"

#ifdef SINGLETON_BASE
ThorQ::Server g_server;
std::set<ThorQ::Account*> g_accounts;
#else
extern ThorQ::Server g_server;
extern std::set<ThorQ::Account*> g_accounts;
#endif

#endif // SINGETONS_H
