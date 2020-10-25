#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <cstdint>
#include <cstdlib>
#include "typedefs_global.h"

namespace ThorQ {
namespace Memory {
ENetCallbacks Initialize();
void DeInitialize();
[[nodiscard("Ignoring return value will result in a memory leak")]] ENetPacket* packetGet(std::size_t size, std::uint32_t flags);
void packetFree(ENetPacket* packet);
}
}

#endif // MEMORYPOOL_H
