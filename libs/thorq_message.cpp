#include "thorq_message.h"


bool __thorq_message::operator ==(const __thorq_message& other) const
{
	return memcmp(this, &other, sizeof(__thorq_message)) == 0;
}

bool __thorq_message::operator !=(const __thorq_message& other) const
{
	return memcmp(this, &other, sizeof(__thorq_message)) != 0;
}
