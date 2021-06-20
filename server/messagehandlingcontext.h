#ifndef MESSAGEHANDLINGCONTEXT_H
#define MESSAGEHANDLINGCONTEXT_H

#include "typedefs_server.h"

#include <schemas_common.h>
#include <typedefs_global.h>

#include <flatbuffers/flatbuffers.h>

#include <vector>
#include <cstdint>
#include <exception>

namespace ThorQ {
class HandlerContext {
public:
    HandlerContext(std::shared_ptr<ThorQ::ApiServerConnection> apiConnection);
    ~HandlerContext();

    inline std::shared_ptr<ThorQ::ApiServerConnection> apiConnection() const
    {
        return m_apiConnection;
    }

    constexpr flatbuffers::FlatBufferBuilder& fbsBuilder()
    {
        return m_fbsBuilder;
    }

    constexpr std::uint64_t requestId() const noexcept
    {
        return m_requestId;
    }
    constexpr void setRequestId(std::uint64_t requestId) noexcept
    {
        m_requestId = requestId;
    }

    template<typename T>
    constexpr const T* body() const noexcept
    {
        return static_cast<const T*>(m_body);
    }
    constexpr void setBody(const void* body) noexcept
    {
        m_body = body;
    }

    void addMessage(ThorQ::Serialization::Body bodyType, flatbuffers::Offset<void> body);
    void sendErrorMessage(const char* error);

    bool sendData();
    bool sendDataUnencrypted();
private:
    std::shared_ptr<ThorQ::ApiServerConnection> m_apiConnection;
    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> m_messages;
    flatbuffers::FlatBufferBuilder m_fbsBuilder;
    std::uint64_t m_requestId;
    const void* m_body;
};

class MessageHandlingException : public std::exception {
public:
    MessageHandlingException(const char* msg, std::uint64_t requestId) noexcept;
    const char* what() const noexcept;
    std::uint64_t requestId() const noexcept;
private:
    const char* m_message;
    std::uint64_t m_requestId;
};
}

#endif // MESSAGEHANDLINGCONTEXT_H
