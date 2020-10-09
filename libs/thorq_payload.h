#ifndef THORQ_PAYLOAD_H
#define THORQ_PAYLOAD_H

#include <memory>
#include <cstdint>

namespace ThorQ {
class Payload
{
    Payload(std::size_t size);
    ~Payload();

    const std::uint8_t* data() const;
    std::size_t size() const;

    void Resize(std::size_t newSize);
private:
    void* m_ptr;
};
}

#endif // THORQ_PAYLOAD_H
