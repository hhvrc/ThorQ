#include "memorymanager.h"

#include <array>

#include <enet.h>
#include <constants.h>

#include "concurrentqueue.h"

moodycamel::ConcurrentQueue<ENetPacket*> g_packetPool;


ENetPacket *packetGetEnet(const void *data, size_t dataLength, enet_uint32 flags)
{
    ENetPacket *packet = ThorQ::Memory::packetGet(dataLength, flags);

    if (packet != nullptr)
    {
        if (flags & ENET_PACKET_FLAG_NO_ALLOCATE)
        {
            packet->data = (std::uint8_t*)data;
        }
        else
        {
            packet->data = (std::uint8_t*)packet + sizeof(ENetPacket);

            if (data != nullptr)
            {
                memcpy(packet->data, data, dataLength);
            }
        }
    }

    return packet;
}

ENetCallbacks ThorQ::Memory::Initialize()
{
    ENetCallbacks callbacks;

    callbacks.free = free;
    callbacks.malloc = malloc;
    callbacks.packet_create = packetGetEnet;
    callbacks.packet_destroy = ThorQ::Memory::packetFree;

    return callbacks;
}

void ThorQ::Memory::DeInitialize()
{

}

ENetPacket* ThorQ::Memory::packetGet(std::size_t size, std::uint32_t flags)
{
    if (size < THORQ_PAYLOAD_LEN_MAX)
    {
        ENetPacket* packet;

        // Get or allocate packet
        if (!g_packetPool.try_dequeue(packet))
        {
            packet = (ENetPacket *)malloc(sizeof(ENetPacket) + THORQ_PAYLOAD_LEN_MAX);
            if (packet == nullptr)
            {
                return nullptr;
            }
        }

        // Initialize struct
        new(packet) ENetPacket;

        // Set rest of data
        packet->referenceCount = 0;
        packet->flags        = flags;
        packet->dataLength   = size;

        return packet;
    }

    return nullptr;
}
void ThorQ::Memory::packetFree(ENetPacket *packet)
{
    if (packet != nullptr)
    {
        packet->~_ENetPacket();
        g_packetPool.enqueue(packet);
    }
}
