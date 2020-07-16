#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstdint>
#include <cstring>
#include <vector>

#include "enums.h"
#include "constants.h"

class Message
{
    std::uint8_t m_data[MESSAGE_DATA_MAX];
    std::size_t  m_size;
public:
    static Message PingMessage();
    static Message Deserialize(const std::vector<std::uint8_t>& data);
    static Message Deserialize(const std::uint8_t* data, std::size_t size);

    Message();
    ~Message();

    bool IsEncrypted() const;
    bool IsCorrectVersion() const;

    const std::uint8_t* data() const;
    std::size_t dataSize() const;

    void SetHeader(ThorQ::MessageHeaderEnums header);
    ThorQ::MessageHeaderEnums Header() const;

    std::size_t payloadMaxSize() const;
    void SetPayload(const std::uint8_t* data, std::size_t size);
    const std::uint8_t* payload() const;
    std::size_t payloadSize() const;
};

#endif // MESSAGE_H
