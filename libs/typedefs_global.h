#ifndef TYPEDEFS_GLOBAL_H
#define TYPEDEFS_GLOBAL_H

typedef struct _ENetPeer ENetPeer;
typedef struct _ENetHost ENetHost;
typedef struct _ENetEvent ENetEvent;
typedef struct _ENetPacket ENetPacket;
typedef struct _ENetAddress ENetAddress;
typedef struct _ENetCallbacks ENetCallbacks;

namespace ThorQ {

class Crypto;

namespace Serialization {

class Heartbeat;
class Version;

namespace Crypto {

class Message;

} // ThorQ::Serialization::Crypto
} // ThorQ::Serialization
} // ThorQ

namespace flatbuffers {
class Verifier;
}

#endif // TYPEDEFS_GLOBAL_H
