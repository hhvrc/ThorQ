#ifndef BLOCK_H
#define BLOCK_H

#include <uuid.h>

#include "user.h"

struct Block
{
	uuids::uuid id;
	User* sender;
	User* receiver;
};

#endif // BLOCK_H
