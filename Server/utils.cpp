#include "utils.h"

#include <vector>

#include <enet.h>
#include <thorq_message.h>
#include <thorq_payload.h>

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

void broadcastNotification(const thorq_payload_t* payload, bool reliable)
{

    std::vector<ThorQ::Instance*> instances = registeredInstances->instances();

	std::vector<std::uint8_t> data;
	thorq_payload_pack(*payload, data);
	thorq_message_encode(data);

    for (ThorQ::Instance* instance : instances)
		instance->sendMessage(data, true, reliable);
}
void broadcastAnnouncement(const thorq_payload_t* payload, bool reliable)
{
	std::vector<std::uint8_t> data;
	thorq_payload_pack(*payload, data);
	thorq_message_encode(data);

    enet_host_broadcast(server, reliable ? 0 : 1, enet_packet_create(data.data(), data.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}
