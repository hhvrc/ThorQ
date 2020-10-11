#ifndef THORQ_PAYLOAD_H
#define THORQ_PAYLOAD_H

#include <memory>
#include <atomic>
#include <cstdint>

namespace ThorQ {
class Payload
{
public:
    static Payload* Create(std::size_t size);
    static void Destroy(Payload* payload);
    ~Payload();

    const std::uint8_t* data() const;
    std::size_t size() const;

    void Resize(std::size_t newSize);
private:
    std::size_t m_size;
    std::atomic<std::uint32_t> m_refcount;
};
}

#endif // THORQ_PAYLOAD_H
