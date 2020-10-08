#include "utils.h"

#include <QDebug>
#include <QSqlDatabase>

#include <enet.h>
#include <thorq_message.h>

#include "singletons.h"
#include "instance.h"

QString enet_peer_address_str(const ENetPeer* addr)
{
	char buffer[50];
    if (enet_address_get_host_ip(&addr->address, buffer, std::size(buffer)) < 0)
		return "ERROR";
    return buffer;
}

QSqlDatabase GetDB(bool readonly)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName("database.db");
    if (readonly) db.setConnectOptions("QSQLITE_OPEN_READONLY");
    return db;
}
