#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include "typedefs.h"
#include "singletons.h"

std::string enetaddr_to_str(const ENetAddress* addr);
void broadcastNotification(std::vector<std::uint8_t>& message, bool reliable = true);
void broadcastAnnouncement(std::vector<std::uint8_t>& message, bool reliable = true);

#endif // UTILS_H
