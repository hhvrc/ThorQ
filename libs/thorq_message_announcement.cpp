#include "thorq_message_announcement.h"

bool __thorq_announcement::operator ==(const __thorq_announcement& other) const
{
	return memcmp(this, &other, sizeof(__thorq_announcement)) == 0;
}

bool __thorq_announcement::operator !=(const __thorq_announcement& other) const
{
	return memcmp(this, &other, sizeof(__thorq_announcement)) != 0;
}

void thorq_msg_announcement_encode(const thorq_announcement_t* announcement, thorq_message_t* msg)
{

}

void thorq_msg_announcement_decode(const thorq_message_t* msg, thorq_announcement_t* announcement)
{

}
