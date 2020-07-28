#ifndef SINGETONS_H
#define SINGETONS_H

namespace ThorQ { class InstanceMap; }
typedef struct _ENetHost ENetHost;

ENetHost* server = nullptr;
ThorQ::InstanceMap* registeredInstances = nullptr;

#endif // SINGETONS_H
