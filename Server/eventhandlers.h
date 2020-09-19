#ifndef EVENTHANDLERS_H
#define EVENTHANDLERS_H

#include <typedefs.h>

void handleEventNewConnection(ENetPeer* peer);
void handleEventMessage(ENetPeer* peer, ENetPacket* packet);
void handleEventDisconnect(ENetPeer* peer);
void handleEventTimeout(ENetPeer* peer);

#endif // EVENTHANDLERS_H
