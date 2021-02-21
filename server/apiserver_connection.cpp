#include "apiserver_connection.h"

#include <cryptography/signer.h>
#include <networking/tcpconnection.h>

#include <systemid.h>
#include <encoding.h>
#include <enums.h>
#include <utils.h>

#include <schemas_common.h>

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
    setAccount(nullptr);
    setSystemID(nullptr);
}

std::shared_ptr<ThorQ::Account> ThorQ::ApiServerConnection::account() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_account));
    return m_account;
}

void ThorQ::ApiServerConnection::setAccount(std::shared_ptr<ThorQ::Account> account)
{
    std::unique_lock l(l_account);
    m_account = account;
}

std::shared_ptr<std::vector<uint8_t> > ThorQ::ApiServerConnection::systemID() const
{
    std::shared_lock l(const_cast<std::shared_mutex&>(l_systemID));
    return m_systemID;
}

void ThorQ::ApiServerConnection::setSystemID(std::shared_ptr<std::vector<uint8_t> > systemID)
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

    for (const auto& fbsMessage : *fbsMessageBuffer->body()) {
        handleMessage(fbsMessage);
    }
}

void ThorQ::ApiServerConnection::onCryptoEstablished()
{
    fmt::print("[CONNECTION] Crypto Established\n");
}

void ThorQ::ApiServerConnection::handleMessage(const void* body)
{
    auto fbsMessage = reinterpret_cast<const ThorQ::Serialization::Message*>(body);
    if (fbsMessage == nullptr) {
        disconnect();
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        handleMessageAccount(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_announcement:
        handleMessageAnnouncement(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_device:
        handleMessageDevice(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_crypto:
        handleMessageCrypto(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_file:
        handleMessageFile(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_friend_request:
        handleMessageFriendRequest(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_group:
        handleMessageGroup(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_moderation:
        handleMessageModeration(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_system_id:
        handleMessageSystemID(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_user:
        handleMessageUser(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(fbsMessage->body());
        break;
    case ThorQ::Serialization::Body_p2p:
        handleMessageP2P(fbsMessage->body());
        break;
    default:
        fmt::print("[MSG] Invalid\n");
        disconnect();
        return;
    }
}

void ThorQ::ApiServerConnection::handleMessageAccount(const void* body)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::Account::Message*>(body);
    if (fbsAccount == nullptr) {
        disconnect();
        return;
    }

    switch (fbsAccount->body_type()) {
    case ThorQ::Serialization::Account::Body_get_account_id:
        handleMessageAccount_GetAccountId(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_get_hashing_parameters:
        handleMessageAccount_GetHashingParameters(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_login_request:
        handleMessageAccount_LoginRequest(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_registration_request:
        handleMessageAccount_RegistrationRequest(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_recover:
        break;
    case ThorQ::Serialization::Account::Body_delete_:
        break;
    case ThorQ::Serialization::Account::Body_logout:
        break;
    case ThorQ::Serialization::Account::Body_set_username:
        break;
    case ThorQ::Serialization::Account::Body_set_password:
        break;
    case ThorQ::Serialization::Account::Body_set_email:
        break;
    case ThorQ::Serialization::Account::Body_set_image:
        break;
    default:
        fmt::print("[MSG][ACCOUNT] Invalid\n");
        disconnect();
        return;
    }/*
    std::string username, password;
    thorq_payload_login_get_username(message, username);
    thorq_payload_login_get_password(message, password);

    if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDOUT)
    {
        auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [&](const std::shared_ptr<ThorQ::Account> account) -> bool
        {
            return account->username() == username;
        });

        if (it != g_accounts.end())
        {
            qDebug() << username << "logged in";

            instance->setAccount(*it);
            instance->setLoginState(THORQ_STATE_LOGIN_LOGGEDIN);

            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_OK);
            instance->packetSend(response, true, true);
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_DENIED);

            instance->packetSend(response, true, true);
        }
    }
    else
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGIN, THORQ_PAYLOAD_ACK_NO_CHANGE);
        instance->packetSend(response, true, true);
    }*/
}
void ThorQ::ApiServerConnection::handleMessageAccount_GetAccountId(const void* body)
{
    auto fbsGetAccountId = reinterpret_cast<const ThorQ::Serialization::Account::GetAccountId*>(body);
    if (fbsGetAccountId == nullptr || fbsGetAccountId->username() == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] GetAccountId\n");
    auto fbsUsername = fbsGetAccountId->username();

    std::string username(fbsUsername->data(), fbsUsername->size());

    fmt::print("[ACCOUNT] Client requested ID for: {}\n", username);
    auto account = ThorQ::Account::GetAccount(username);

    if (account == nullptr) {
        fmt::print("[ACCOUNT] Failed to find account, creating fake one...\n");
        account = ThorQ::Account::NewAccount(username);
    }

    if (account == nullptr) {
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        disconnect();
        return;
    }

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsRespAccountID = fbsBuilder.CreateStruct(ThorQ::Serialization::Uuid(account->id().toBytes())).Union();
    auto fbsRespAccount   = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_account_id, fbsRespAccountID).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsRespAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
void ThorQ::ApiServerConnection::handleMessageAccount_GetHashingParameters(const void* body)
{
    auto fbsGetHashingParameters = reinterpret_cast<const ThorQ::Serialization::Account::GetHashingParameters*>(body);
    if (fbsGetHashingParameters == nullptr || fbsGetHashingParameters->account_id() == nullptr || fbsGetHashingParameters->account_id()->data() == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] GetHashingParameters\n");
    auto fbsAccountId = fbsGetHashingParameters->account_id()->data();
    ThorQ::Uuid accountId(std::span<const std::uint8_t, 16>(fbsAccountId->data(), fbsAccountId->size()));

    auto account = ThorQ::Account::GetAccount(accountId);

    ThorQ::Crypto::Hashing::HashingParameters parameters;

    if (account == nullptr) {
        // If account doesnt exist then create fake hashing parameters to keep exploiter confused
        // Create fake hashing parameters (and make them use max performance because why tf not (Make them suffer x3))
        parameters.setPerformance(ThorQ::Crypto::Hashing::HashingParameters::Performance::Sensitive);
    }
    else {
        parameters = account->passwordHashParameters();
    }

    fmt::print("[ACCOUNT] Sending HashingParameters: {} {} {}\n", parameters.ops_limit, parameters.mem_limit, parameters.algorithm);

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsRespHashPrms  = fbsBuilder.CreateStruct(ThorQ::Serialization::Account::HashingParameters(parameters.ops_limit, parameters.mem_limit, parameters.algorithm)).Union();
    auto fbsRespAccount   = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_hashing_parameters, fbsRespHashPrms).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsRespAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
void ThorQ::ApiServerConnection::handleMessageAccount_LoginRequest(const void* body)
{
    auto fbsLoginRequest = reinterpret_cast<const ThorQ::Serialization::Account::LoginRequest*>(body);
    if (fbsLoginRequest == nullptr || fbsLoginRequest->account_id() == nullptr || fbsLoginRequest->account_id()->data() == nullptr || fbsLoginRequest->password_hash() == nullptr || fbsLoginRequest->password_hash()->hash() == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] LoginRequest\n");
    auto fbsAccountId = fbsLoginRequest->account_id()->data();
    auto fbsPasswordHash = fbsLoginRequest->password_hash()->hash();
    ThorQ::Uuid accountId(std::span<const std::uint8_t, 16>(fbsAccountId->data(), fbsAccountId->size()));

    auto account = ThorQ::Account::GetAccount(accountId);

    flatbuffers::FlatBufferBuilder fbsBuilder;

    flatbuffers::Offset<void> fbsLoginResp;
    ThorQ::Serialization::Account::AuthToken authToken;
    if (account != nullptr && memcmp(fbsPasswordHash->data(), account->passwordHash().data(), ThorQ::Crypto::Hashing::HashLength) == 0) {
        setAccount(account);
        // TODO: Create auth token
        if (fbsLoginRequest->get_auth_token()) {
            // TODO: Get or create auth token
            fbsLoginResp = ThorQ::Serialization::Account::CreateLoginResponse(fbsBuilder, true, &authToken).Union();
        }
        else {
            fbsLoginResp = ThorQ::Serialization::Account::CreateLoginResponse(fbsBuilder, true).Union();
        }
    }
    else {
        fmt::print("[ACCOUNT] Login request DENIED\n");
        fbsLoginResp = ThorQ::Serialization::Account::CreateLoginResponse(fbsBuilder, false).Union();
    }

    auto fbsRespAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_login_response, fbsLoginResp).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsRespAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
void ThorQ::ApiServerConnection::handleMessageAccount_RegistrationRequest(const void* body)
{
    auto fbsRegistrationReq = reinterpret_cast<const ThorQ::Serialization::Account::RegistrationRequest*>(body);
    if (fbsRegistrationReq == nullptr || fbsRegistrationReq->username() == nullptr || fbsRegistrationReq->email() == nullptr || fbsRegistrationReq->password_hash() == nullptr || fbsRegistrationReq->password_hash()->hash() == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] GetAccountId\n");
    auto fbsUsername = fbsRegistrationReq->username();
    auto fbsEmail    = fbsRegistrationReq->email();

    std::string username(fbsUsername->data(), fbsUsername->size());
    std::string email(fbsEmail->data(), fbsEmail->size());

    fmt::print("[ACCOUNT] Client requested ID for: {}\n", username);
    auto account = ThorQ::Account::GetAccount(username);

    if (account == nullptr) {
        fmt::print("[ACCOUNT] Failed to find account, creating fake one...\n");
        account = ThorQ::Account::NewAccount(username);
    }

    if (account == nullptr) {
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] RegistrationRequest\n");
}

void ThorQ::ApiServerConnection::handleMessageAnnouncement(const void* body)
{
    auto fbsAnnouncement = reinterpret_cast<const ThorQ::Serialization::Announcement::Message*>(body);
    if (fbsAnnouncement == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[MSG] Announcement\n");
}

void ThorQ::ApiServerConnection::handleMessageDevice(const void* body)
{
    auto fbsDevice = reinterpret_cast<const ThorQ::Serialization::Device::Message*>(body);
    if (fbsDevice == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[MSG] Device\n");

    /*
    if (instance->sessionState() == THORQ_STATE_SESSION_ACTIVE)
        if (instance->partner() != nullptr)
            instance->partner()->sendMessage(message, true, false);
    */
}

void ThorQ::ApiServerConnection::handleMessageCrypto(const void* body)
{
    auto fbsCrypto = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(body);
    if (fbsCrypto == nullptr) {
        disconnect();
        return;
    }

    auto fbsClientPublicKey = fbsCrypto->public_key();

    if (!m_crypto.generateKeyPair()) {
        fmt::print(stderr, "[CRYPTO] Failed to generate keypair!\n");
        return;
    }

    if (m_crypto.setForeignKey(fbsClientPublicKey->data(), fbsClientPublicKey->size()))
    {
        auto serverPublickey = m_crypto.publicKey();

        ThorQ::Crypto::Signer signer;
        signer.generateKeyPair();

        // TODO: load this at server startup
        if (!signer.tryLoadFromFile("root_signing.pksk")) {
            fmt::print(stderr, "[CRYPTO] Failed to load signer keypair!\n");
            return;
        }

        // PUBLIC_CLIENT_KEY + PUBLIC_SERVER_KEY
        std::array<std::uint8_t, ThorQ::Crypto::Signer::PublicKeyLen * 2> combinedPublicKeys;
        memcpy(combinedPublicKeys.data(), fbsClientPublicKey->data(), ThorQ::Crypto::Encryption::PublicKeyLen);
        memcpy(combinedPublicKeys.data() + ThorQ::Crypto::Encryption::PublicKeyLen, serverPublickey.data(), ThorQ::Crypto::Encryption::PublicKeyLen);

        // SIGNATURE(PUBLIC_CLIENT_KEY + PUBLIC_SERVER_KEY)
        std::array<std::uint8_t, ThorQ::Crypto::Signer::SignatureLen> signature;
        if (!signer.sign(combinedPublicKeys, signature)) {
            fmt::print(stderr, "[CRYPTO] Failed to sign encryption public key!\n");
            return;
        }

        // Build flatbuffer
        flatbuffers::FlatBufferBuilder fbsBuilder;
        auto fbsPublicKey = fbsBuilder.CreateVector(serverPublickey.data(), serverPublickey.size());
        auto fbsSignature = fbsBuilder.CreateVector(signature.data(), signature.size());
        auto fbsCrypto    = ThorQ::Serialization::Crypto::CreateMessage(fbsBuilder, fbsPublicKey, fbsSignature).Union();

        std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
        messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_crypto, fbsCrypto));
        fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));

        if (encodeAndSend(fbsBuilder.GetBufferSpan(), false)) {
            onCryptoEstablished();
        }
    }
    else
    {
        fmt::print(stderr, "[CRYPTO] Got key with invalid length!\n");
        disconnect(); // TODO: In the future should find a way to send reason for disconnet to server as well
    }
}

void ThorQ::ApiServerConnection::handleMessageFile(const void* body)
{
    auto fbsFile = reinterpret_cast<const ThorQ::Serialization::File::Message*>(body);
    if (fbsFile == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[MSG] File\n");
}

void ThorQ::ApiServerConnection::handleMessageFriendRequest(const void* body)
{
    auto fbsFriendRequest = reinterpret_cast<const ThorQ::Serialization::FriendRequest::Message*>(body);
    if (fbsFriendRequest == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[MSG] Friend request\n");

    /*
    std::vector<std::uint8_t> response;

    THORQ_COMMAND_ID cmd;
    thorq_payload_command_get_id(message, cmd);

    if (instance->authState() != THORQ_STATE_AUTH_OK)
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_UNAUTHORIZED);
        instance->packetSend(response, false, true);
        return;
    }

    switch (cmd){
    case THORQ_COMMAND_ID_GET_USER_LIST:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_OK);
            instance->packetSend(response, true, true);

            std::vector<ThorQ::Instance*> instances = g_sessions.toList();

            for (ThorQ::Instance* i : instances)
            {
                if (i->account() != nullptr)
                {
                    thorq_payload_notification_pack(response, THORQ_NOTIFICATION_USER_ACTIVITY, i->account()->username(), i->activityState());
                    instance->packetSend(response, true, true);
                }
            }
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_REQUEST:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string username;
            thorq_payload_command_get_data(message, username);

            auto it = std::find_if(g_accounts.begin(), g_accounts.end(), [&](const std::shared_ptr<ThorQ::Account> account) -> bool
            {
                return account->username() == username;
            });

            if (*it == nullptr)
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_COMMAND_ID_SESSION_REQUEST, THORQ_PAYLOAD_ACK_DENIED, username + "is not an account");
                instance->packetSend(response, true);
                return;
            }

            std::unordered_setThorQ::Instance*> targetInstances = (*it)->instances();

            if (targetInstances.isEmpty())
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_SESSION, THORQ_PAYLOAD_ACK_DENIED, username + "is not online");
                instance->packetSend(response, true);
                return;
            }

            for (ThorQ::Instance* otherInstance : (*it)->instances())
            instance->requestOn(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_ACCEPT:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string name;
            thorq_payload_command_get_data(message, name);

            ThorQ::Instance* otherInstance = g_sessions->get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_COMMAND, cmd, THORQ_PAYLOAD_ACK_DENIED, name + "is not online");
                instance->packetSend(response, true);
                return;
            }

            instance->requestAcceptFrom(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_PAYLOAD_ROOM_
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::string name;
            thorq_payload_
            thorq_payload_command_get_data(message, name);

            auto sit = std::find_if(g_accounts.begin(), g_accounts.end(), [name](const std::shared_ptr<ThorQ::Account> a) -> bool
            {
                if (a == nullptr) return false;

                return a->username() == name;
            });
            ThorQ::Instance* otherInstance = g_sessions .get(name);

            if (otherInstance == nullptr)
            {
                thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_DENIED, name + "is not online");
                instance->packetSend(response, true);
                return;
            }

            instance->requestDenyFrom(otherInstance);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SESSION_LEAVE:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            instance->setSessionState(THORQ_STATE_SESSION_NONE);

            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_OK);
            instance->packetSend(response, true, true);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    case THORQ_COMMAND_ID_SET_SELF_STATE:
    {
        if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
        {
            std::uint8_t state;
            thorq_payload_command_get_data(message, state);
            instance->setActivityState(state);
        }
        else
        {
            thorq_payload_ack_pack(response, cmd, THORQ_PAYLOAD_ACK_LOGIN_NEEDED);
            instance->packetSend(response, true, true);
        }
        break;
    }
    }
    */
}

void ThorQ::ApiServerConnection::handleMessageGroup(const void* body)
{
    auto fbsGroup = reinterpret_cast<const ThorQ::Serialization::Group::Message*>(body);

    fmt::print("[MSG] Group\n");

    /*
    std::vector<std::uint8_t> response;

    if (instance->loginState() == THORQ_STATE_LOGIN_LOGGEDIN)
    {
        instance->setLoginState(THORQ_STATE_LOGIN_LOGGEDOUT);

        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGOUT, THORQ_PAYLOAD_ACK_OK);
        instance->packetSend(response, true, true);
    }
    else
    {
        thorq_payload_ack_pack(response, THORQ_PAYLOAD_ID_ACCOUNT, THORQ_PAYLOAD_ACCOUNT_LOGOUT, THORQ_PAYLOAD_ACK_NO_CHANGE);
        instance->packetSend(response, true, true);
    }
    */
}

void ThorQ::ApiServerConnection::handleMessageModeration(const void* body)
{
    auto fbsModeration = reinterpret_cast<const ThorQ::Serialization::Moderation::Message*>(body);

    fmt::print("[MSG] Moderation\n");
}

void ThorQ::ApiServerConnection::handleMessageSystemID(const void* body)
{
    auto fbsSystemID = reinterpret_cast<const ThorQ::Serialization::SystemId::Message*>(body);

    if (fbsSystemID->cmd() != ThorQ::Serialization::SystemId::Command_Submit)
    {
        // TODO: THORQ_DISCONNECT_REASON::INVALID_OPERATION
        disconnect();
        return;
    }

    std::span<std::uint8_t> systemid(const_cast<std::uint8_t*>(fbsSystemID->data()->data()), fbsSystemID->data()->size());

    if (!ThorQ::SystemID::systemid_validate(systemid))
    {
        fmt::print("[MSG] Got forged SystemID!\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_HWID
        disconnect();
        return;
    }

    std::string systemID = ThorQ::SystemID::systemid_to_string(systemid);

    fmt::print("SystemID: {}\n", systemID);

    auto dbConnection = SQLite::Connection::OpenConnection("database.db", SQLite::Connection::READWRITE);
    if (dbConnection == nullptr)
    {
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        disconnect();
        return;
    }

    dbConnection->setBusyTimeout(5000);

    auto dbInsertSystemId = dbConnection->makeQuery("INSERT OR IGNORE INTO systems(hardware_id) VALUES (?);"sv);
    dbInsertSystemId.bindText(1, systemID);

    if (!dbInsertSystemId.step())
    {
        fmt::print(stderr, "[SQLITE] Failed to insert system: {}\n", dbConnection->lastError());
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        disconnect();
        return;
    }

    auto dbCheckSystemIdBanned = dbConnection->makeQuery(
                "SELECT COUNT(*) FROM accounts WHERE account_id IN ("
                "SELECT account_id FROM account_systems WHERE system_id IN ("
                "SELECT system_id FROM systems WHERE hardware_id = ?"
                ")) AND banned_at IS NOT NULL LIMIT 1"sv);
    dbCheckSystemIdBanned.bindText(1, systemID);

    if (!dbCheckSystemIdBanned.step())
    {
        fmt::print(stderr, "[SQLITE] Failed to select banned_at: {}\n", dbConnection->lastError());
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        disconnect();
        return;
    }

    if (dbCheckSystemIdBanned.columnCount() == 0)
    {
        fmt::print(stderr, "[SQLITE] SQLite query returned invalid amount of rows ({})\n", dbCheckSystemIdBanned.columnCount());
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        disconnect();
        return;
    }

    bool isBanned = (dbCheckSystemIdBanned.column(0).getInt32() > 0);

    if (isBanned)
    {
        fmt::print("Connection is banned!\n");
        // TODO: THORQ_DISCONNECT_REASON::BANNED
        disconnect();
        return;
    }

    fmt::print("Connection is ok!\n");
}

void ThorQ::ApiServerConnection::handleMessageUser(const void* body)
{
    auto fbsUser = reinterpret_cast<const ThorQ::Serialization::User::Message*>(body);

    fmt::print("[MSG] User\n");
}

void ThorQ::ApiServerConnection::handleMessageVersion(const void* body)
{
    auto fbsVersion = reinterpret_cast<const ThorQ::Serialization::Version*>(body);

    fmt::print("[MSG] version\n");

    ThorQ::Version version;
    version.setMajor(fbsVersion->major());
    version.setMinor(fbsVersion->minor());
    version.setPatch(fbsVersion->patch());

    switch ((THORQ_APP)fbsVersion->app()) {
    case THORQ_APP::SERVER:
        if (version == ThorQ::ServerVersion) {
            fmt::print("[VERSION] Server version matched\n");
        }
        else {
            fmt::print("[VERSION] Server version mismatched\n");
            // TODO: THORQ_DISCONNECT_REASON::VERSION_MISMATCH
            disconnect();
        }
        break;
    case THORQ_APP::CLIENT:
        if (version == ThorQ::ClientVersion) {
            fmt::print("[VERSION] Client version matched\n");
        }
        else {
            fmt::print("[VERSION] Client version mismatched\n");
            // TODO: THORQ_DISCONNECT_REASON::VERSION_MISMATCH
            disconnect();
        }
        break;
    case THORQ_APP::LINK:
        if (version == ThorQ::LinkVersion) {
            fmt::print("[VERSION] Link version matched\n");
        }
        else {
            fmt::print("[VERSION] Link version mismatched\n");
            // TODO: THORQ_DISCONNECT_REASON::VERSION_MISMATCH
            disconnect();
        }
        break;
    default:
        fmt::print("[VERSION] invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        disconnect();
        return;
    }
}

void ThorQ::ApiServerConnection::handleMessageP2P(const void* body)
{
    auto fbsP2P = reinterpret_cast<const ThorQ::Serialization::Peer2Peer::Message*>(body);

    fmt::print("[MSG] P2P\n");
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
