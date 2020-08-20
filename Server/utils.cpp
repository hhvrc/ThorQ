#include "utils.h"

#include <vector>

#include <enet.h>
#include <thorq_message.h>

#include "singletons.h"
#include "instance.h"
#include "instancemap.h"

std::string enetaddr_to_str(const ENetAddress* addr)
{
	char buffer[50];
	if (enet_address_get_host_ip(addr, buffer, sizeof(buffer)) < 0)
		return "ERROR";
	return std::string(buffer);
}

void broadcastNotification(std::vector<std::uint8_t>& message, bool reliable)
{

    std::vector<ThorQ::Instance*> instances = registeredInstances->instances();

    thorq_message_encode(message);

    for (ThorQ::Instance* instance : instances)
        instance->sendMessage(message, true, reliable);
}
void broadcastAnnouncement(std::vector<std::uint8_t>& message, bool reliable)
{
    thorq_message_encode(message);

    enet_host_broadcast(server, reliable ? 0 : 1, enet_packet_create(message.data(), message.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}
