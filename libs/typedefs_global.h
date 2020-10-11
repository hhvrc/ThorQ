#ifndef TYPEDEFS_GLOBAL_H
#define TYPEDEFS_GLOBAL_H

class QTimer;
class QThread;
class QSqlDatabase;
class QElapsedTimer;

typedef struct _ENetPeer ENetPeer;
typedef struct _ENetHost ENetHost;
typedef struct _ENetEvent ENetEvent;
typedef struct _ENetPacket ENetPacket;
typedef struct _ENetAddress ENetAddress;
typedef struct _ENetCallbacks ENetCallbacks;

namespace ThorQ {
class Crypto;
class Payload;
}

#endif // TYPEDEFS_GLOBAL_H
