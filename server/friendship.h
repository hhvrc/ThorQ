#ifndef FRIENDSHIP_H
#define FRIENDSHIP_H

#include <stduuid/include/uuid.h>

#include <enums.h>

#include "account.h"

namespace ThorQ {
class FriendShip
{
	// Stored as: db_id, uuid, source, target, allow_control_without_requests
public:
private:
	int m_dbId;
    uuids::uuid m_uuid;

	bool m_accepted = false;

    std::shared_ptr<ThorQ::Account> m_friend;
	THORQ_SESSION_AUTHORITY m_sessionAuthority = THORQ_SESSION_AUTHORITY_PROMPT; ///< How
};
}

#endif // FRIENDSHIP_H
