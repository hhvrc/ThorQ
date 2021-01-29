#ifndef MESSAGE_H
#define MESSAGE_H

#include "typedefs_global.h"

#include <vector>
#include <memory>
#include <cstdint>

namespace ThorQ {
namespace Networking {

#pragma pack(push, 1)
struct MessageHeader
{
    MessageHeader() : size(0), checkSum(0) {}
    MessageHeader(std::uint16_t size_, std::uint16_t checkSum_) : size(size_), checkSum(checkSum_) {}

    std::uint32_t size;
    std::uint32_t checkSum;
};
#pragma pack(pop)

struct Message
{
    std::shared_ptr<MessageHeader> header;
    std::shared_ptr<std::vector<std::uint8_t>> body;
};

struct IncomingMessage : public Message
{
    std::shared_ptr<ThorQ::Networking::Connection> remote;
};
}
}

#endif // MESSAGE_H
