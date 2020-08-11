#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "typedefs.h"
#include "singletons.h"

std::string enetaddr_to_str(const ENetAddress* addr);
void broadcastNotification(const thorq_payload_t* payload, bool reliable = true);
void broadcastAnnouncement(const thorq_payload_t* payload, bool reliable = true);

#endif // UTILS_H
