#ifndef MESSAGEHANDLINGCONTEXT_H
#define MESSAGEHANDLINGCONTEXT_H

#include <schemas/message_generated.h>

#include <flatbuffers/flatbuffers.h>

#include <vector>
#include <cstdint>
#include <exception>

namespace ThorQ {
struct HandlerContext {
    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    flatbuffers::FlatBufferBuilder fbsBuilder;
    std::uint64_t requestId;
    const void* body;
    bool encrypt;
};
class MessageHandlingException : public std::exception {
    const char* m_message;
    std::uint64_t m_requestId;
public:
    MessageHandlingException(const char* msg, std::uint64_t requestId) noexcept : std::exception(), m_message(msg), m_requestId(requestId){}
    constexpr const char* what() const noexcept override { return m_message; }
    constexpr std::uint64_t requestId() const noexcept { return m_requestId; }
};
}

#endif // MESSAGEHANDLINGCONTEXT_H
