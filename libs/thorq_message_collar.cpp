#include "thorq_message_collar.h"

bool __thorq_collar::operator ==(const __thorq_collar& other) const
{
	return memcmp(this, &other, sizeof(__thorq_collar)) == 0;
}

bool __thorq_collar::operator !=(const __thorq_collar& other) const
{
	return memcmp(this, &other, sizeof(__thorq_collar)) != 0;
}
