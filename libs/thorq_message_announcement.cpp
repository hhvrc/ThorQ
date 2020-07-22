#include "thorq_message_announcement.h"

bool __thorq_announcement::operator ==(const __thorq_announcement& other) const
{
	return memcmp(this, &other, sizeof(__thorq_announcement)) == 0;
}

bool __thorq_announcement::operator !=(const __thorq_announcement& other) const
{
	return memcmp(this, &other, sizeof(__thorq_announcement)) != 0;
}
