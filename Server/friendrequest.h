#ifndef FRIENDREQUEST_H
#define FRIENDREQUEST_H

#include <QUuid>

#include "account.h"

namespace ThorQ {
struct FriendRequest
{
	int dbId;
	QUuid uuid;
	Account* sender;
	Account* receiver;
};
}

#endif // FRIENDREQUEST_H
