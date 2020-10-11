#include "thorq_payload.h"

#include <cstdlib>
#include <cstring>
#include <atomic>
#include <memory>


MetaData* getMeta(void* ptr)
{
    return (MetaData*)ptr - 1;
}
const MetaData* getMeta(const void* ptr)
{
    return (MetaData*)ptr - 1;
}
std::uint8_t* getData(void* ptr)
{
    return (std::uint8_t*)ptr;
}
const std::uint8_t* getData(const void* ptr)
{
    return (std::uint8_t*)ptr;
}

ThorQ::Payload::Payload(std::size_t size)
{
    m_ptr = (std::uint8_t*)malloc(sizeof(MetaData) + size) + sizeof(MetaData);

    // Set metadata
    MetaData* meta = (MetaData*)m_ptr;
    new(meta) MetaData();

    m_ptr = data;
}

ThorQ::Payload* ThorQ::Payload::Create(size_t size)
{
    ThorQ::Payload* payload = (ThorQ::Payload*)malloc(sizeof(MetaData) + size);
}

void ThorQ::Payload::Destroy(ThorQ::Payload* payload)
{

}

ThorQ::Payload::~Payload()
{
    MetaData* meta = getMeta(m_ptr);

    if (meta->refcount.fetch_sub(1) < 1)
    {
        meta->~MetaData();
        free(meta);
    }
}

const std::uint8_t *ThorQ::Payload::data() const
{
    return (std::uint8_t*)m_ptr;
}

std::size_t ThorQ::Payload::size() const
{
    return *((std::size_t*)(data() - sizeof(std::size_t)));
}


void ThorQ::Payload::Resize(std::size_t newSize)
{

}



THORQ_PAYLOAD_ID ThorQ::_THORQ_PAYLOAD::payloadId() const
{
    if (m_id > THORQ_PAYLOAD_ID__MAX)
    {
        return THORQ_PAYLOAD_ID__INVALID;
    }

    return static_cast<THORQ_PAYLOAD_ID>(m_id);
}
void ThorQ::_THORQ_PAYLOAD::setPayloadId(THORQ_PAYLOAD_ID payloadId)
{
    m_id = payloadId;
}
ThorQ::_THORQ_PAYLOAD::PayloadData &ThorQ::_THORQ_PAYLOAD::data()
{
    return m_data;
}
const ThorQ::_THORQ_PAYLOAD::PayloadData &ThorQ::_THORQ_PAYLOAD::data() const
{
    return m_data;
}
uint32_t ThorQ::_THORQ_PAYLOAD::dataSize() const
{
    return ntohl(m_size);
}
void ThorQ::_THORQ_PAYLOAD::setDataSize(uint32_t size)
{
    m_size = htonl(size);
}
