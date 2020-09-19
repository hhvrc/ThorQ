#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <QSet>
#include <QMap>
#include <QByteArray>
#include <cstdint>

#include <constants.h>

namespace ThorQ {
namespace AuthHandler {
bool tryAddRegkey(const QByteArray& key);
void removeRegkey(const QByteArray& key);

enum ResponseCode
{
	REGISTERED,
	RE_REGISTERED,
	NOT_REGISTERED,

	TIMEOUT,
	INVALID_REGKEY,
	INVALID_SYSTEMID,
};
ResponseCode checkSystemID(const QByteArray& hwid);
ResponseCode tryRegisterSystemID(const QByteArray& hwid, const QByteArray& key);
}
}

#endif // AUTHHANDLER_H
