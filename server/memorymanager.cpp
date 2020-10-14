#include "memorymanager.h"

#include <array>

#include <enet.h>
#include <constants.h>

#include "concurrentqueue.h"

moodycamel::ConcurrentQueue<ENetPacket*> g_packets;

ENetCallbacks Initialize()
{
    ENetCallbacks callbacks;

    callbacks.free = free;
    callbacks.malloc = malloc;
    callbacks.packet_create = ThorQ::Memory::packetGet;
    callbacks.packet_destroy = ThorQ::Memory::packetFree;

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

ENetPacket* ThorQ::Memory::packetGet(const void* data, std::size_t dataLength, std::uint32_t flags)
{
    if (dataLength < THORQ_PAYLOAD_LEN_MAX)
    {
        ENetPacket* packet;

        // Get or allocate packet
        if (!g_packets.try_dequeue(packet))
        {
            packet = (ENetPacket *)malloc(sizeof(ENetPacket) + THORQ_PAYLOAD_LEN_MAX);
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
void ThorQ::Memory::packetFree(ENetPacket *packet)
{
    if (packet != nullptr)
    {
        packet->~_ENetPacket();
        g_packets.enqueue(packet);
    }
}
