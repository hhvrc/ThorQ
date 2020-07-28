#ifndef SINGETONS_H
#define SINGETONS_H

typedef struct _ENetHost ENetHost;
namespace ThorQ { class InstanceMap; }

ENetHost* server;
ThorQ::InstanceMap* registeredInstances{};

#endif // SINGETONS_H
