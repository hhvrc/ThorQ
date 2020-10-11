#include "memorymanager.h"

#include <array>

#include "enet.h"
#include "concurrentqueue.h"

constexpr std::size_t BUFFER_SIZE = 1024 * 64;

moodycamel::ConcurrentQueue<ENetPacket*> g_packetsTX;
moodycamel::ConcurrentQueue<ENetPacket*> g_packetsRX;

moodycamel::ConcurrentQueue<ENetPacket*> g_packetsAvailable;

ENetPacket* packetGet(const void *data, size_t dataLength, enet_uint32 flags)
{
    if (dataLength <= BUFFER_SIZE)
    {
        ENetPacket* packet;

        // Get or allocate packet
        if (!g_packetsAvailable.try_dequeue(packet))
        {
            packet = (ENetPacket *)malloc(sizeof(ENetPacket) + BUFFER_SIZE);
            if (packet == nullptr)
            {
                return nullptr;
            }
        }

        // Initialize struct
        new(packet) ENetPacket;

        // Set data
        if (data != nullptr)
        {
            memcpy(packet->data, data, dataLength);
        }

        // Set rest of data
        packet->referenceCount = 0;
        packet->flags        = flags;
        packet->dataLength   = dataLength;

        return packet;
    }

    return nullptr;
}
void packetFree(ENetPacket* packet)
{
    if (packet != nullptr)
    {
        packet->~_ENetPacket();
        g_packetsAvailable.enqueue(packet);
    }
}
ENetPacket* workerAlloc()
{

}
void workerFree()
{

}

ENetCallbacks Initialize()
{
    ENetCallbacks callbacks;

    callbacks.packet_create = packetGet;
    callbacks.packet_destroy = packetFree;

    return callbacks;
}

ENetPacket *enet_packet_create(const void *data, size_t dataLength, enet_uint32 flags) {
    ENetPacket *packet;
    if (flags & ENET_PACKET_FLAG_NO_ALLOCATE) {
        packet = (ENetPacket *)enet_malloc(sizeof (ENetPacket));
        if (packet == NULL) {
            return NULL;
        }

        packet->data = (enet_uint8 *)data;
    }
    else {
        packet = (ENetPacket *)enet_malloc(sizeof (ENetPacket) + dataLength);
        if (packet == NULL) {
            return NULL;
        }

        packet->data = (enet_uint8 *)packet + sizeof(ENetPacket);

        if (data != NULL) {
            memcpy(packet->data, data, dataLength);
        }
    }

    packet->referenceCount = 0;
    packet->flags        = flags;
    packet->dataLength   = dataLength;
    packet->freeCallback = NULL;
    packet->userData     = NULL;

    return packet;
}
