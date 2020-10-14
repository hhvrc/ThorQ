#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <cstdint>
#include <cstdlib>
#include "typedefs_global.h"

namespace ThorQ {
namespace Memory {
ENetCallbacks Initialize();
ENetPacket* packetGet(const void* data, std::size_t dataLength, std::uint32_t flags);
void packetFree(ENetPacket* packet);
}
}

#endif // MEMORYPOOL_H
