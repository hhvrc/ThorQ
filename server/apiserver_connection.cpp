#include "apiserver_connection.h"

#include <cryptography/signer.h>
#include <networking/tcpconnection.h>

#include <systemid.h>
#include <encoding.h>
#include <enums.h>
#include <utils.h>

#include <schemas_common.h>

#include "database.h"

#include <lsql/transaction.h>
#include <lsql/connection.h>
#include <lsql/column.h>
#include <lsql/query.h>
#include <fmt/core.h>

#include <cstring>

using namespace std::literals;

ThorQ::ApiServerConnection::ApiServerConnection(asio::io_context& asio, asio::ip::tcp::socket socket)
    : ThorQ::Networking::TcpConnection(asio, std::move(socket))
    , m_buffer(THORQ_PAYLOAD_LEN_TYP)
    , m_crypto()
    , l_account()
    , m_account(nullptr)
    , l_systemID()
    , m_systemID(nullptr)
{
}

ThorQ::ApiServerConnection::ApiServerConnection(ThorQ::ApiServerConnection&& other)
    : ThorQ::Networking::TcpConnection(std::move(other))
    , m_buffer(std::move(other.m_buffer))
    , m_crypto(std::move(other.m_crypto))
    , l_account()
    , m_account(std::move(other.m_account))
    , l_systemID()
    , m_systemID(std::move(other.m_systemID))
{
}

ThorQ::ApiServerConnection::~ApiServerConnection()
{
    disconnect();
}

bool ThorQ::ApiServerConnection::encodeAndSend(flatbuffers::span<std::uint8_t> buffer, bool encrypt)
{
    auto message = std::make_shared<std::vector<std::uint8_t>>();
    message->resize(ThorQ::Encoding::calculateMessageSize(buffer.size(), encrypt));

    if (encrypt) {
        if (!ThorQ::Encoding::messageEncode(buffer.data(), buffer.size(), message->data(), message->size(), m_crypto)) {
            fmt::print(stderr, "Failed to encrypt message\n");
            return false;
        }
    }
    else {
        if (!ThorQ::Encoding::messageEncode(buffer.data(), buffer.size(), message->data(), message->size())) {
            fmt::print(stderr, "Failed to encode message\n");
            return false;
        }
    }

    messageSend(message);

    return true;
}


std::shared_ptr<ThorQ::Account> ThorQ::ApiServerConnection::account() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_account));
    return m_account;
}

void ThorQ::ApiServerConnection::setAccount(std::shared_ptr<ThorQ::Account> account)
{
    account->addInstance(std::static_pointer_cast<ThorQ::ApiServerConnection>(shared_from_this()));

    std::unique_lock l(l_account);
    m_account = account;
}

std::shared_ptr<std::vector<uint8_t> > ThorQ::ApiServerConnection::systemID() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_systemID));
    return m_systemID;
}

void ThorQ::ApiServerConnection::setSystemID(std::shared_ptr<std::vector<std::uint8_t>> systemID)
{
    std::unique_lock l(l_systemID);
    m_systemID = systemID;
}

void ThorQ::ApiServerConnection::onError(std::error_code ec)
{
    fmt::print(stderr, "[CONNECTION] Error: {}\n", ec.message());
}

void ThorQ::ApiServerConnection::onConnect(std::vector<std::uint8_t> address, std::uint16_t port)
{
    fmt::print("[CONNECTION] Connected\n");
}

void ThorQ::ApiServerConnection::onDisconnect()
{
    fmt::print("[CONNECTION] Disconnected\n");
}

bool ThorQ::ApiServerConnection::onHeader(const ThorQ::Encoding::MessageHeader* header)
{
    return ThorQ::Encoding::isHeaderValid(header);
}

void ThorQ::ApiServerConnection::onMessage(std::shared_ptr<std::vector<std::uint8_t>> message)
{
    if (!ThorQ::Encoding::isMessageValid(message->data(), message->size()))
    {
        fmt::print(stderr, "Received packet is corrupted\n");
        disconnect();
        return;
    }

    m_buffer.resize(ThorQ::Encoding::calculateDataSize(message->data(), message->size()));
    if (!ThorQ::Encoding::messageDecode(message->data(), message->size(), m_buffer.data(), m_buffer.size(), m_crypto))
    {
        fmt::print(stderr, "Cannot decode/decrypt packet\n");
        disconnect();
        return;
    }

    auto fbsMessageBuffer = flatbuffers::GetRoot<ThorQ::Serialization::MessageBuffer>(m_buffer.data());
    auto fbsVerifier = flatbuffers::Verifier(m_buffer.data(), m_buffer.size());

    if (!fbsMessageBuffer->Verify(fbsVerifier))
    {
        fmt::print(stderr, "Invalid flatbuffer message\n");
        disconnect();
        return;
    }

    if (fbsMessageBuffer->body() == nullptr) {
        disconnect();
        return;
    }

    auto& messages = *fbsMessageBuffer->body();

    HandlerContext context(std::static_pointer_cast<ThorQ::ApiServerConnection>(shared_from_this()));

    // Catch any thrown exceptions
    try {

        // Iterate trough all received requests
        for (const auto& fbsMessage : messages) {

            if (fbsMessage == nullptr) {
                disconnect();
                return;
            }

            // Set context body to message
            context.setBody(fbsMessage);

            // Handle the message
            handleMessage(context);
        }
    } catch (MessageHandlingException& ex) {
        // If a message exception is thrown, send the exception message to the user
        context.sendErrorMessage(ex.what());
    } catch (std::exception& ex) {
        // If a general exception is thrown then do not display it to the user
        fmt::print(stderr, "Exception while parsing messages: {}\n", ex.what());
        context.sendErrorMessage("Internal server error");
    } catch (...) {
        // I have no idea why this would be thrown, but send an error to the user
        fmt::print(stderr, "Unknown exception while parsing messages!\n");
        context.sendErrorMessage("Internal server error");
    }

    // Send all the queued data to the user
    context.sendData();
}

void ThorQ::ApiServerConnection::handleMessage(HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Message>();
    auto subBody = msgBody->body();
    auto connection = context.apiConnection();

    if (subBody == nullptr) {
        connection->disconnect();
    }

    context.setBody(subBody);
    context.setRequestId(msgBody->request_id());

    fmt::print("[MSG] {}\n", context.requestId());

    switch (msgBody->body_type()) {
    case ThorQ::Serialization::Body_account:
        ThorQ::ApiEndpoints::AccountEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_announcement:
        ThorQ::ApiEndpoints::AnnouncementEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_device:
        ThorQ::ApiEndpoints::DeviceEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_crypto:
        ThorQ::ApiEndpoints::CryptoEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_file:
        ThorQ::ApiEndpoints::FileEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_friend_request:
        ThorQ::ApiEndpoints::FriendRequestEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_group:
        ThorQ::ApiEndpoints::GroupEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_moderation:
        ThorQ::ApiEndpoints::ModerationEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_system_id:
        ThorQ::ApiEndpoints::SystemidEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_user:
        ThorQ::ApiEndpoints::UserEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_version:
        ThorQ::ApiEndpoints::VersionEndpoint::handleMessage(context);
        break;
    case ThorQ::Serialization::Body_p2p:
        ThorQ::ApiEndpoints::P2PEndpoint::handleMessage(context);
        break;
    default:
        fmt::print("[MSG] Invalid\n");
        disconnect();
        return;
    }
}
