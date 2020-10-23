#ifndef FRIENDSHIP_H
#define FRIENDSHIP_H

#include <QObject>
#include <QUuid>

#include <enums.h>

#include "account.h"

namespace ThorQ {
class FriendShip : public QObject
{
	Q_OBJECT
	// Stored as: db_id, uuid, source, target, allow_control_without_requests
public:
private:
	int m_dbId;
	QUuid m_uuid;

	bool m_accepted = false;

    std::shared_ptr<ThorQ::Account> m_friend;
	THORQ_SESSION_AUTHORITY m_sessionAuthority = THORQ_SESSION_AUTHORITY_PROMPT; ///< How
};
}

#endif // FRIENDSHIP_H
