#include "thorq_message.h"

#include "enums.h"

__thorq_msg::__thorq_msg()
{

}

__thorq_msg::__thorq_msg(const __thorq_msg &other)
{
    flags = other.flags;
    payload = other.payload;
    memcpy(payload_iv, other.payload_iv, THORQ_CRYPTO_CIPHER_IV_LEN);
}

__thorq_msg& __thorq_msg::operator =(const __thorq_msg &other)
{
    flags = other.flags;
    payload = other.payload;
    memcpy(payload_iv, other.payload_iv, THORQ_CRYPTO_CIPHER_IV_LEN);
    return *this;
}

bool __thorq_msg::operator ==(const __thorq_msg& other) const
{
    return flags == other.flags &&
           payload == other.payload &&
           (memcmp(payload_iv, other.payload_iv, THORQ_CRYPTO_CIPHER_IV_LEN) == 0);
}

bool __thorq_msg::operator !=(const __thorq_msg& other) const
{
    return !(*this == other);
}

bool thorq_msg_is_valid(const std::uint8_t* data, std::size_t size)
{
    if (data == nullptr || size <= THORQ_MSG_SIZE_MIN || size > THORQ_MSG_SIZE_MAX)
        return false;

    if ((data[0] & THORQ_MSG_FLAG_ENCRYPTED) != 0)
        return size > 1 + THORQ_CRYPTO_CIPHER_IV_LEN;

    return size > 1;
}

bool thorq_msg_is_encrypted(const thorq_msg_t* msg)
{
    return (msg->flags & THORQ_MSG_FLAG_ENCRYPTED) != 0;
}

bool thorq_msg_encrypt(thorq_msg_t* msg, ThorQ::Crypto* crypto)
{
    if ((msg->flags & THORQ_MSG_FLAG_ENCRYPTED) == 0)
    {
        if (!crypto->encrypt(msg->payload, msg->payload_iv))
            return false;
        msg->flags |= THORQ_MSG_FLAG_ENCRYPTED;
    }
    return true;
}

bool thorq_msg_decrypt(thorq_msg_t* msg, ThorQ::Crypto* crypto)
{
    if ((msg->flags & THORQ_MSG_FLAG_ENCRYPTED) != 0)
    {
        if (!crypto->decrypt(msg->payload, msg->payload_iv))
            return false;
        msg->flags &= ~THORQ_MSG_FLAG_ENCRYPTED;
    }
    return true;
}


bool thorq_msg_encode(const thorq_msg_t* msg, std::vector<std::uint8_t>* data)
{
    if (msg->payload.size() == 0)
        return false;

    data->resize(THORQ_MSG_SIZE_MIN + msg->payload.size());

    std::uint8_t* ptr = data->data();

    memcpy(ptr, msg->payload_iv, sizeof(thorq_msg_t::payload_iv));
    ptr += sizeof(thorq_msg_t::payload_iv);

    memcpy(ptr, msg->payload.data(), msg->payload.size());
    ptr += msg->payload.size();

    memcpy(ptr, &msg->flags, sizeof(thorq_msg_t::flags));
    ptr += sizeof(thorq_msg_t::flags);

    return true;
}

bool thorq_msg_decode(const uint8_t *data, std::size_t size, thorq_msg_t* msg)
{
    if (!thorq_msg_is_valid(data, size))
        return false;

    msg->payload.resize(size - THORQ_MSG_SIZE_MIN);

    memcpy(msg->payload_iv, data, sizeof(thorq_msg_t::payload_iv));
    data += sizeof(thorq_msg_t::payload_iv);

    memcpy(msg->payload.data(), data, msg->payload.size());
    data += msg->payload.size();

    memcpy(&msg->flags, data, sizeof(thorq_msg_t::flags));
    data += sizeof(thorq_msg_t::flags);

    return true;
}

uint8_t thorq_msg_get_msg_id(const thorq_msg_t *msg)
{
    if (msg->payload.size() < sizeof(std::uint8_t))
        return 0;

    return msg->payload.data()[0];
}
