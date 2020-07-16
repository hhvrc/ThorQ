#include "message.h"

Message Message::PingMessage()
{
    Message msg;
    msg.SetHeader(ThorQ::MessageHeaderEnums::HEADER_HEARTBEAT);
    return msg;
}

Message::Message()
    : m_data()
    , m_size(0)
{
    memset(m_data, 0, MESSAGE_DATA_MAX);
    m_data[0] = VERSION_MAJOR;
    m_data[1] = VERSION_MINOR;
    m_data[2] = VERSION_PATCH;
}

Message::~Message()
{
}

bool Message::IsEncrypted() const
{
    return m_data[3] > ThorQ::MessageHeaderEnums::HEADER_CRYPT_ESTABLISH;
}

bool Message::IsCorrectVersion() const
{
    return m_data[0] == VERSION_MAJOR
        && m_data[1] == VERSION_MINOR
        && m_data[2] == VERSION_PATCH;
}

const uint8_t* Message::data() const
{
    return m_data;
}

std::size_t Message::dataSize() const
{
    return m_size + MESSAGE_HEADER_SIZE;
}

void Message::SetHeader(ThorQ::MessageHeaderEnums header)
{
    m_data[3] = header;
}

ThorQ::MessageHeaderEnums Message::Header() const
{
    return ThorQ::MessageHeaderEnums(m_data[3]);
}

void Message::SetPayload(const uint8_t *data, std::size_t size)
{
    memcpy(m_data + MESSAGE_HEADER_SIZE, data, size);
}

const uint8_t *Message::payload() const
{
    return m_data + MESSAGE_HEADER_SIZE;
}

std::size_t Message::payloadSize() const
{
    return m_size;
}

std::size_t Message::payloadMaxSize() const
{
    return MESSAGE_PAYLOAD_MAX;
}
