#ifndef SINGLETONS_H
#define SINGLETONS_H

#include "typedefs_server.h"

#ifdef SINGLETON_BASE
ThorQ::Server* g_server = nullptr;
#else
extern ThorQ::Server* g_server;
#endif

#endif // SINGLETONS_H
