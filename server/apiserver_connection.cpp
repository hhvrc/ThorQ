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

    auto& messages = *fbsMessageBuffer->body();

    HandlerContext context;

    // Response will be encrypted by default
    context.encrypt = true;

    // Catch any thrown exceptions
    try {

        // Iterate trough all received requests
        for (const auto& fbsMessage : messages) {

            // Set context body, and if its nullptr, then client is bad
            context.body = fbsMessage;
            if (context.body == nullptr) {
                disconnect();
                return;
            }

            // Handle the message
            handleMessage(context);
        }
    } catch (MessageHandlingException& ex) {
        // If a message exception is thrown, send the exception message to the user
        createErrorMessage(context, ex.what(), ex.requestId());
    } catch (std::exception& ex) {
        // If a general exception is thrown then do not display it to the user
        fmt::print(stderr, "Exception while parsing messages: {}\n", ex.what());
        createErrorMessage(context, nullptr, 0);
    } catch (...) {
        // I have no idea why this would be thrown, but send an error to the user
        fmt::print(stderr, "Unknown exception while parsing messages!\n");
        createErrorMessage(context, nullptr, 0);
    }

    // Send all the queued data to the user
    sendContextData(context);
}

void ThorQ::ApiServerConnection::onCryptoEstablished()
{
    fmt::print("[CONNECTION] Crypto Established\n");
    crypto_ok = true;
}

void ThorQ::ApiServerConnection::handleMessage(HandlerContext& context)
{
    auto fbsMessage = reinterpret_cast<const ThorQ::Serialization::Message*>(context.body);

    context.body = fbsMessage->body();
    if (context.body == nullptr) {
        disconnect();
        return;
    }

    switch (fbsMessage->body_type()) {
    case ThorQ::Serialization::Body_account:
        handleMessageAccount(context);
        break;
    case ThorQ::Serialization::Body_announcement:
        handleMessageAnnouncement(context);
        break;
    case ThorQ::Serialization::Body_device:
        handleMessageDevice(context);
        break;
    case ThorQ::Serialization::Body_crypto:
        handleMessageCrypto(context);
        break;
    case ThorQ::Serialization::Body_file:
        handleMessageFile(context);
        break;
    case ThorQ::Serialization::Body_friend_request:
        handleMessageFriendRequest(context);
        break;
    case ThorQ::Serialization::Body_group:
        handleMessageGroup(context);
        break;
    case ThorQ::Serialization::Body_moderation:
        handleMessageModeration(context);
        break;
    case ThorQ::Serialization::Body_system_id:
        handleMessageSystemID(context);
        break;
    case ThorQ::Serialization::Body_user:
        handleMessageUser(context);
        break;
    case ThorQ::Serialization::Body_version:
        handleMessageVersion(context);
        break;
    case ThorQ::Serialization::Body_p2p:
        handleMessageP2P(context);
        break;
    default:
        fmt::print("[MSG] Invalid\n");
        disconnect();
        return;
    }
}

bool ThorQ::ApiServerConnection::sendContextData(ThorQ::ApiServerConnection::HandlerContext &context)
{
    auto fbsRespBuffer = ThorQ::Serialization::CreateMessageBufferDirect(context.fbsBuilder, &context.messages);
    context.fbsBuilder.Finish(fbsRespBuffer);

    bool result = encodeAndSend(context.fbsBuilder.GetBufferSpan(), context.encrypt);

    context.messages.clear();
    context.fbsBuilder.Clear();
    context.encrypt = true;

    return result;
}

void ThorQ::ApiServerConnection::createErrorMessage(ThorQ::ApiServerConnection::HandlerContext& context, const char* error, std::uint64_t requestId)
{
    // Removes all other data
    context.messages.clear();
    context.fbsBuilder.Clear();
    context.encrypt = false;

    auto fbsRespError     = ThorQ::Serialization::CreateErrorDirect(context.fbsBuilder, requestId, error).Union();
    auto fbsRespMessage   = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_error, fbsRespError);
    context.messages.push_back(fbsRespMessage);
}

void ThorQ::ApiServerConnection::handleMessageAccount(HandlerContext& context)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::Account::Message*>(context.body);

    context.body = fbsAccount->body();
    if (context.body == nullptr) {
        disconnect();
        return;
    }

    switch (fbsAccount->body_type()) {
    case ThorQ::Serialization::Account::Body_get_account_id:
        handleMessageAccount_GetAccountId(context);
        break;
    case ThorQ::Serialization::Account::Body_get_hashing_salt:
        handleMessageAccount_GetHashingSalt(context);
        break;
    case ThorQ::Serialization::Account::Body_get_hashing_parameters:
        handleMessageAccount_GetHashingParameters(context);
        break;
    case ThorQ::Serialization::Account::Body_login_request:
        handleMessageAccount_LoginRequest(context);
        break;
    case ThorQ::Serialization::Account::Body_registration_request:
        handleMessageAccount_RegistrationRequest(context);
        break;
    case ThorQ::Serialization::Account::Body_recover:
        handleMessageAccount_Recover(context);
        break;
    case ThorQ::Serialization::Account::Body_delete_:
        handleMessageAccount_Delete(context);
        break;
    case ThorQ::Serialization::Account::Body_logout:
        handleMessageAccount_Logout(context);
        break;
    case ThorQ::Serialization::Account::Body_set_username:
        handleMessageAccount_SetUserName(context);
        break;
    case ThorQ::Serialization::Account::Body_set_password:
        handleMessageAccount_SetPassword(context);
        break;
    case ThorQ::Serialization::Account::Body_set_email:
        handleMessageAccount_SetEmail(context);
        break;
    case ThorQ::Serialization::Account::Body_set_image:
        handleMessageAccount_SetImage(context);
        break;
    default:
        fmt::print("[MSG][ACCOUNT] Invalid\n");
        disconnect();
        return;
    }
}
void ThorQ::ApiServerConnection::handleMessageAccount_GetAccountId(HandlerContext& context)
{
    auto fbsGetAccountId = reinterpret_cast<const ThorQ::Serialization::Account::GetAccountId*>(context.body);

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

    auto fbsRespAccountID = context.fbsBuilder.CreateStruct(ThorQ::Serialization::Uuid(account->id().toBytes())).Union();
    auto fbsRespAccount   = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_account_id, fbsRespAccountID).Union();
    auto fbsRespMessage   = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsRespAccount);
    context.messages.push_back(fbsRespMessage);
}

void ThorQ::ApiServerConnection::handleMessageAccount_GetHashingSalt(ThorQ::ApiServerConnection::HandlerContext& context)
{
    auto fbsGetHashingParameters = reinterpret_cast<const ThorQ::Serialization::Account::GetHashingSalt*>(context.body);
    if (fbsGetHashingParameters->account_id() == nullptr || fbsGetHashingParameters->account_id()->data() == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] GetHashingSalt\n");
    auto fbsAccountId = fbsGetHashingParameters->account_id()->data();
    ThorQ::Uuid accountId(std::span<const std::uint8_t, 16>(fbsAccountId->data(), fbsAccountId->size()));

    auto account = ThorQ::Account::GetAccount(accountId);

    ThorQ::Crypto::Hashing::Salt salt;

    if (account == nullptr) {
        // If account doesnt exist then create fake hashing salt to keep exploiter confused
        randombytes_buf(salt.data(), ThorQ::Crypto::Hashing::SaltLength);
    }
    else {
        salt = account->passwordSalt();
    }

    auto fbsRespSalt      = context.fbsBuilder.CreateStruct(ThorQ::Serialization::Account::HashingSalt(salt)).Union();
    auto fbsRespAccount   = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_hashing_salt, fbsRespSalt).Union();
    auto fbsRespMessage   = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsRespAccount);
    context.messages.push_back(fbsRespMessage);
}

void ThorQ::ApiServerConnection::handleMessageAccount_GetHashingParameters(HandlerContext& context)
{
    auto fbsGetHashingParameters = reinterpret_cast<const ThorQ::Serialization::Account::GetHashingParameters*>(context.body);
    if (fbsGetHashingParameters->account_id() == nullptr || fbsGetHashingParameters->account_id()->data() == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] GetHashingParameters\n");
    auto fbsAccountId = fbsGetHashingParameters->account_id()->data();
    ThorQ::Uuid accountId(std::span<const std::uint8_t, 16>(fbsAccountId->data(), fbsAccountId->size()));

    auto account = ThorQ::Account::GetAccount(accountId);

    ThorQ::Crypto::Hashing::Parameters parameters;

    if (account == nullptr) {
        // If account doesnt exist then create fake hashing parameters to keep exploiter confused
        // Create fake hashing parameters (and make them use max performance because why tf not (Make them suffer x3))
        parameters.setPerformance(ThorQ::Crypto::Hashing::Parameters::Performance::Sensitive);
    }
    else {
        parameters = account->passwordHashParameters();
    }

    fmt::print("[ACCOUNT] Sending HashingParameters: {} {} {}\n", parameters.ops_limit, parameters.mem_limit, parameters.algorithm);

    auto fbsRespHashPrms  = context.fbsBuilder.CreateStruct(ThorQ::Serialization::Account::HashingParameters(parameters.ops_limit, parameters.mem_limit, parameters.algorithm)).Union();
    auto fbsRespAccount   = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_hashing_parameters, fbsRespHashPrms).Union();
    auto fbsRespMessage   = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsRespAccount);
    context.messages.push_back(fbsRespMessage);
}
void ThorQ::ApiServerConnection::handleMessageAccount_LoginRequest(HandlerContext& context)
{
    auto fbsLoginRequest = reinterpret_cast<const ThorQ::Serialization::Account::LoginRequest*>(context.body);
    if (fbsLoginRequest == nullptr || fbsLoginRequest->account_id() == nullptr || fbsLoginRequest->account_id()->data() == nullptr || fbsLoginRequest->password_hash() == nullptr || fbsLoginRequest->password_hash()->hash() == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] LoginRequest\n");
    auto fbsAccountId = fbsLoginRequest->account_id()->data();
    auto fbsPasswordHash = fbsLoginRequest->password_hash()->hash();

    ThorQ::Uuid accountId(std::span<const std::uint8_t, 16>(fbsAccountId->data(), fbsAccountId->size()));
    ThorQ::Crypto::Hashing::Hash passwordHash;
    memcpy(passwordHash.data(), fbsPasswordHash->data(), ThorQ::Crypto::Hashing::HashLength);

    auto account = ThorQ::Account::GetAccount(accountId);

    flatbuffers::Offset<void> fbsRespLogin;
    ThorQ::Serialization::Uuid fbsRespAccountID;
    ThorQ::Serialization::Account::AuthToken fbsRespAuthToken;
    if (account != nullptr && account->checkPasswordHash(passwordHash)) {
        setAccount(account);

        fbsRespAccountID = ThorQ::Serialization::Uuid(account->id().toBytes());

        // TODO: Create auth token
        if (fbsLoginRequest->get_auth_token()) {
            // TODO: Get auth token
            fbsRespLogin = ThorQ::Serialization::Account::CreateLoginResponse(context.fbsBuilder, true, &fbsRespAccountID, &fbsRespAuthToken).Union();
        }
        else {
            fbsRespLogin = ThorQ::Serialization::Account::CreateLoginResponse(context.fbsBuilder, true, &fbsRespAccountID).Union();
        }
    }
    else {
        fmt::print("[ACCOUNT] Login request DENIED\n");
        fbsRespLogin = ThorQ::Serialization::Account::CreateLoginResponse(context.fbsBuilder, false).Union();
    }

    auto fbsRespAccount = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_login_response, fbsRespLogin).Union();
    auto fbsRespMessage = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsRespAccount);
    context.messages.push_back(fbsRespMessage);
}
void ThorQ::ApiServerConnection::handleMessageAccount_RegistrationRequest(HandlerContext& context)
{
    auto fbsRegistrationReq = reinterpret_cast<const ThorQ::Serialization::Account::RegistrationRequest*>(context.body);
    if (fbsRegistrationReq->account_id() == nullptr || fbsRegistrationReq->account_id()->data() == nullptr || fbsRegistrationReq->email() == nullptr || fbsRegistrationReq->password_hash() == nullptr || fbsRegistrationReq->password_hash()->hash() == nullptr) {
        disconnect();
        return;
    }

    fmt::print("[ACCOUNT] RegistrationRequest\n");
    auto& fbsAccountId     = *fbsRegistrationReq->account_id()->data();
    auto& fbsEmail         = *fbsRegistrationReq->email();
    auto& fbsPasswordHash  = *fbsRegistrationReq->password_hash()->hash();
    auto& fbsHashingParams =  fbsRegistrationReq->password_hash()->params();

    ThorQ::Uuid accountID(std::span<const std::uint8_t, 16>(fbsAccountId.data(), fbsAccountId.size()));
    std::string email(fbsEmail.data(), fbsEmail.size());

    ThorQ::Crypto::Hashing::Hash passwordHash;
    memcpy(passwordHash.data(), fbsPasswordHash.data(), ThorQ::Crypto::Hashing::HashLength);

    ThorQ::Crypto::Hashing::Parameters hashingParams;
    hashingParams.mem_limit = fbsHashingParams.mem_limit();
    hashingParams.ops_limit = fbsHashingParams.ops_limit();
    hashingParams.algorithm = fbsHashingParams.algorithm();

    fmt::print("[ACCOUNT] Client requested account: {}\n", accountID.toString());
    auto account = ThorQ::Account::GetAccount(accountID);

    if (account == nullptr) {
        throw MessageHandlingException("AccountID not found", 1); // TODO implement requestID's
    }

    if (!account->tryClaim(email, passwordHash, hashingParams)) {
        fmt::print("[ACCOUNT] {} already taken!\n", account->username());
        // TODO respond with username/email taken
        return;
    }

    fmt::print("[ACCOUNT] {} claimed!\n", account->username());
}

void ThorQ::ApiServerConnection::handleMessageAccount_Recover(HandlerContext& context)
{

}

void ThorQ::ApiServerConnection::handleMessageAccount_Delete(HandlerContext& context)
{

}

void ThorQ::ApiServerConnection::handleMessageAccount_Logout(HandlerContext& context)
{

}

void ThorQ::ApiServerConnection::handleMessageAccount_SetUserName(HandlerContext& context)
{

}

void ThorQ::ApiServerConnection::handleMessageAccount_SetPassword(HandlerContext& context)
{

}

void ThorQ::ApiServerConnection::handleMessageAccount_SetEmail(HandlerContext& context)
{

}

void ThorQ::ApiServerConnection::handleMessageAccount_SetImage(HandlerContext& context)
{

}

void ThorQ::ApiServerConnection::handleMessageAnnouncement(HandlerContext& context)
{
    auto fbsAnnouncement = reinterpret_cast<const ThorQ::Serialization::Announcement::Message*>(context.body);

    fmt::print("[MSG] Announcement\n");
}

void ThorQ::ApiServerConnection::handleMessageDevice(HandlerContext& context)
{
    auto fbsDevice = reinterpret_cast<const ThorQ::Serialization::Device::Message*>(context.body);

    fmt::print("[MSG] Device\n");

    /*
    if (instance->sessionState() == THORQ_STATE_SESSION_ACTIVE)
        if (instance->partner() != nullptr)
            instance->partner()->sendMessage(message, true, false);
    */
}

void ThorQ::ApiServerConnection::handleMessageCrypto(HandlerContext& context)
{
    auto fbsClientPublicKey = reinterpret_cast<const ThorQ::Serialization::Crypto::Message*>(context.body)->public_key();

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
        auto fbsPublicKey = context.fbsBuilder.CreateVector(serverPublickey.data(), serverPublickey.size());
        auto fbsSignature = context.fbsBuilder.CreateVector(signature.data(), signature.size());
        auto fbsCrypto    = ThorQ::Serialization::Crypto::CreateMessage(context.fbsBuilder, fbsPublicKey, fbsSignature).Union();

        context.messages.push_back(ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_crypto, fbsCrypto));

        context.encrypt = false;
        if (sendContextData(context)) {
            onCryptoEstablished();
        }
    }
    else
    {
        fmt::print(stderr, "[CRYPTO] Got key with invalid length!\n");
        disconnect(); // TODO: In the future should find a way to send reason for disconnet to server as well
    }
}

void ThorQ::ApiServerConnection::handleMessageFile(HandlerContext& context)
{
    auto fbsFile = reinterpret_cast<const ThorQ::Serialization::File::Message*>(context.body);

    fmt::print("[MSG] File\n");
}

void ThorQ::ApiServerConnection::handleMessageFriendRequest(HandlerContext& context)
{
    auto fbsFriendRequest = reinterpret_cast<const ThorQ::Serialization::FriendRequest::Message*>(context.body);

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

void ThorQ::ApiServerConnection::handleMessageGroup(HandlerContext& context)
{
    auto fbsGroup = reinterpret_cast<const ThorQ::Serialization::Group::Message*>(context.body);

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

void ThorQ::ApiServerConnection::handleMessageModeration(HandlerContext& context)
{
    auto fbsModeration = reinterpret_cast<const ThorQ::Serialization::Moderation::Message*>(context.body);

    fmt::print("[MSG] Moderation\n");
}

void ThorQ::ApiServerConnection::handleMessageSystemID(HandlerContext& context)
{
    auto fbsSystemID = reinterpret_cast<const ThorQ::Serialization::SystemId::Message*>(context.body);

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

void ThorQ::ApiServerConnection::handleMessageUser(HandlerContext& context)
{
    auto fbsUser = reinterpret_cast<const ThorQ::Serialization::User::Message*>(context.body);

    fmt::print("[MSG] User\n");
}

void ThorQ::ApiServerConnection::handleMessageVersion(HandlerContext& context)
{
    auto fbsVersion = reinterpret_cast<const ThorQ::Serialization::Version*>(context.body);

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

void ThorQ::ApiServerConnection::handleMessageP2P(HandlerContext& context)
{
    auto fbsP2P = reinterpret_cast<const ThorQ::Serialization::Peer2Peer::Message*>(context.body);

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
