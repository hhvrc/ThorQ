#ifndef UTILS_H
#define UTILS_H

#include <string>

#include <enet.h>
#include <thorq_message.h>
#include <thorq_payload.h>

#include "singletons.h"

std::string enetaddr_to_str(const ENetAddress* addr)
{
	char buffer[50];
	if (enet_address_get_host_ip(addr, buffer, 50) < 0)
		return "ERROR";
	return std::string(buffer);
}
void broadcastPayload(const thorq_payload_t& payload, bool reliable = true)
{
	std::vector<std::uint8_t> message;
	thorq_payload_pack(payload, message);

	std::vector<std::uint8_t> data;
	thorq_message_encode(message, data);

	enet_host_broadcast(server, reliable ? 0 : 1, enet_packet_create(data.data(), data.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}

#endif // UTILS_H
