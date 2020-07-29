#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "typedefs.h"

std::string enetaddr_to_str(const ENetAddress* addr);
void broadcastPayload(const thorq_payload_t* payload, bool reliable = true);

#endif // UTILS_H
