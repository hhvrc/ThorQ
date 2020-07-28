#ifndef EVENTHANDLERS_H
#define EVENTHANDLERS_H

typedef struct _ENetPeer ENetPeer;
typedef struct _ENetPacket ENetPacket;

void handleEventNewConnection(ENetPeer* peer);
void handleEventMessage(ENetPeer* peer, ENetPacket* packet);
void handleEventDisconnect(ENetPeer* peer);
void handleEventTimeout(ENetPeer* peer);

#endif // EVENTHANDLERS_H
