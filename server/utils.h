#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <QString>

#include <typedefs_global.h>

#include "typedefs_server.h"

QString enet_peer_address_str(const ENetPeer* addr);
void broadcastNotification(std::vector<std::uint8_t>& message, bool reliable = true);
void broadcastAnnouncement(std::vector<std::uint8_t>& message, bool reliable = true);

QSqlDatabase GetDB(bool readonly);

#endif // UTILS_H
