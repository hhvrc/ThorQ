#include "messagehandlingcontext.h"

#include "apiserver_connection.h"

#include <schemas_common.h>

ThorQ::HandlerContext::HandlerContext(std::shared_ptr<ThorQ::ApiServerConnection> apiConnection)
    : m_apiConnection(apiConnection)
{
}

ThorQ::HandlerContext::~HandlerContext()
{
}

void ThorQ::HandlerContext::addMessage(ThorQ::Serialization::Body bodyType, flatbuffers::Offset<void> body)
{
    m_messages.push_back(ThorQ::Serialization::CreateMessage(m_fbsBuilder, bodyType, body, m_requestId));
}

void ThorQ::HandlerContext::sendErrorMessage(const char* error)
{
    // Removes all other data
    m_messages.clear();
    m_fbsBuilder.Clear();

    auto fbsRespError     = ThorQ::Serialization::CreateErrorDirect(m_fbsBuilder, m_requestId, error).Union();
    addMessage(ThorQ::Serialization::Body_error, fbsRespError);

    sendDataUnencrypted();
}

bool ThorQ::HandlerContext::sendData()
{
    if (m_messages.empty()) {
        return true;
    }

    auto fbsRespBuffer = ThorQ::Serialization::CreateMessageBufferDirect(m_fbsBuilder, &m_messages);
    m_fbsBuilder.Finish(fbsRespBuffer);

    bool result = m_apiConnection->encodeAndSend(m_fbsBuilder.GetBufferSpan(), true);

    m_messages.clear();
    m_fbsBuilder.Clear();

    return result;
}

bool ThorQ::HandlerContext::sendDataUnencrypted()
{
    if (m_messages.empty()) {
        return true;
    }

    auto fbsRespBuffer = ThorQ::Serialization::CreateMessageBufferDirect(m_fbsBuilder, &m_messages);
    m_fbsBuilder.Finish(fbsRespBuffer);

    bool result = m_apiConnection->encodeAndSend(m_fbsBuilder.GetBufferSpan(), false);

    m_messages.clear();
    m_fbsBuilder.Clear();

    return result;
}

ThorQ::MessageHandlingException::MessageHandlingException(const char* msg, uint64_t requestId) noexcept
    : std::exception(), m_message(msg), m_requestId(requestId)
{
}

const char* ThorQ::MessageHandlingException::what() const noexcept
{
    return m_message;
}

uint64_t ThorQ::MessageHandlingException::requestId() const noexcept
{
    return m_requestId;
}
