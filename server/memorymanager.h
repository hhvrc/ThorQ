#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <cstdlib>
#include "typedefs_global.h"

namespace ThorQ {
namespace Memory {
ENetCallbacks Initialize();
ENetPacket* packetGet(const void *data, size_t dataLength, enet_uint32 flags);
}
}

#endif // MEMORYPOOL_H
