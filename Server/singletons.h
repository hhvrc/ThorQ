#ifndef SINGETONS_H
#define SINGETONS_H

#include "typedefs.h"

#ifdef SINGLETON_BASE
ENetHost* server = nullptr;
ThorQ::InstanceMap* registeredInstances = nullptr;
#else
extern ENetHost* server;
extern ThorQ::InstanceMap* registeredInstances;
#endif

#endif // SINGETONS_H
