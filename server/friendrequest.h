#ifndef FRIENDREQUEST_H
#define FRIENDREQUEST_H

#include <uuid.h>

#include "account.h"

namespace ThorQ {
struct FriendRequest
{
	int dbId;
    uuids::uuid uuid;
    std::shared_ptr<ThorQ::Account> sender;
    std::shared_ptr<ThorQ::Account> receiver;
};
}

#endif // FRIENDREQUEST_H
