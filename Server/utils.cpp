#include "utils.h"

#include <QDebug>
#include <QSqlDatabase>

#include <enet.h>
#include <thorq_message.h>

#include "singletons.h"
#include "session.h"

QString enet_peer_address_str(const ENetPeer* addr)
{
	char buffer[50];
    if (enet_address_get_host_ip(&addr->address, buffer, sizeof(buffer)) < 0)
		return "ERROR";
    return buffer;
}

void broadcastNotification(std::vector<std::uint8_t>& message, bool reliable)
{

    QList<ThorQ::Instance*> instances = g_sessions.values();

    qDebug() << "Sending notification to" << instances.size() <<  "instances!";

    thorq_message_encode(message);

    for (ThorQ::Instance* instance : instances)
        instance->sendMessage(message, true, reliable);
}
void broadcastAnnouncement(std::vector<std::uint8_t>& message, bool reliable)
{
    thorq_message_encode(message);

    enet_host_broadcast(g_server, reliable ? 0 : 1, enet_packet_create(message.data(), message.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED));
}

QSqlDatabase GetDB(bool readonly)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName("database.db");
    if (readonly) db.setConnectOptions("QSQLITE_OPEN_READONLY");
    return db;
}
