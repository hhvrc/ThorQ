#ifndef MESSAGECONTEXT_H
#define MESSAGECONTEXT_H

#include <flatbuffers/flatbuffers.h>

#include <vector>
#include <cstdint>

namespace ThorQ::Serialization {
class Message;
}
struct MessageContext {
    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    flatbuffers::FlatBufferBuilder fbsBuilder;
    std::uint64_t requestId;
    const void* body;
    bool encrypt;
};

#endif // MESSAGECONTEXT_H
