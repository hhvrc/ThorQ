#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <QObject>
#include <QUuid>

#include <enums.h>

#include "account.h"

namespace ThorQ {
struct Relationship
{
	// Stored as: db_id, source_account, target_account, friend_status, block_status, target_authority
	int dbId;

	Account* sourceAccount;
	Account* targetAccount;

	THORQ_RELATIONSHIP_FRIEND friendStatus;       ///< What friend status source has to target
	THORQ_RELATIONSHIP_BLOCK blockStatus;         ///< What block source has applied to target
	THORQ_RELATIONSHIP_AUTHORITY targetAuthority; ///< What authority target has over source
};
}

#endif // RELATIONSHIP_H
