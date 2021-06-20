#include "endpoint_account.h"

#include "account.h"
#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

using namespace std::literals;

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Account::Message>();
    auto subBody = msgBody->body();
    auto connection = context.apiConnection();

    if (subBody == nullptr) {
        connection->disconnect();
        return;
    }

    context.setBody(subBody);

    switch (msgBody->body_type()) {
    case ThorQ::Serialization::Account::Body_get_account_id:
        handleMessageGetAccountId(context);
        break;
    case ThorQ::Serialization::Account::Body_get_hashing_salt:
        handleMessageGetHashingSalt(context);
        break;
    case ThorQ::Serialization::Account::Body_get_hashing_parameters:
        handleMessageGetHashingParameters(context);
        break;
    case ThorQ::Serialization::Account::Body_login_request:
        handleMessageLoginRequest(context);
        break;
    case ThorQ::Serialization::Account::Body_logout_request:
        handleMessageLogoutRequest(context);
        break;
    case ThorQ::Serialization::Account::Body_registration_request:
        handleMessageRegistrationRequest(context);
        break;
    case ThorQ::Serialization::Account::Body_recovery_request:
        handleMessageRecoveryRequest(context);
        break;
    case ThorQ::Serialization::Account::Body_deletion_request:
        handleMessageDeletionRequest(context);
        break;
    case ThorQ::Serialization::Account::Body_update_request:
        handleMessageUpdateRequest(context);
        break;
    case ThorQ::Serialization::Account::Body_set_image_request:
        handleMessageSetImageRequest(context);
        break;
    default:
        fmt::print("[MSG][ACCOUNT] Invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        connection->disconnect();
        break;
    }
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageGetAccountId(ThorQ::HandlerContext& context)
{
    auto fbsGetAccountId = context.body<ThorQ::Serialization::Account::GetAccountId>();
    auto connection = context.apiConnection();

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
        connection->disconnect();
        return;
    }

    auto fbsRespAccountID = context.fbsBuilder().CreateStruct(ThorQ::Serialization::Uuid(account->uuid().toBytes())).Union();
    auto fbsRespAccount   = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder(), ThorQ::Serialization::Account::Body_account_id, fbsRespAccountID).Union();
    context.addMessage(ThorQ::Serialization::Body_account, fbsRespAccount);
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageGetHashingSalt(ThorQ::HandlerContext& context)
{
    auto fbsGetHashingParameters = context.body<ThorQ::Serialization::Account::GetHashingSalt>();
    auto connection = context.apiConnection();

    if (fbsGetHashingParameters->account_id() == nullptr || fbsGetHashingParameters->account_id()->data() == nullptr) {
        connection->disconnect();
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
        if (fbsGetHashingParameters->new_password()) {
            salt = account->newPasswordSalt();
        }
        else {
            salt = account->currentPasswordSalt();
        }
    }

    auto fbsRespSalt      = context.fbsBuilder().CreateStruct(ThorQ::Serialization::Account::HashingSalt(salt)).Union();
    auto fbsRespAccount   = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder(), ThorQ::Serialization::Account::Body_hashing_salt, fbsRespSalt).Union();
    context.addMessage(ThorQ::Serialization::Body_account, fbsRespAccount);
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageGetHashingParameters(ThorQ::HandlerContext& context)
{
    auto fbsGetHashingParameters = context.body<ThorQ::Serialization::Account::GetHashingParameters>();
    auto connection = context.apiConnection();

    if (fbsGetHashingParameters->account_id() == nullptr || fbsGetHashingParameters->account_id()->data() == nullptr) {
        connection->disconnect();
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

    auto fbsRespHashPrms  = context.fbsBuilder().CreateStruct(ThorQ::Serialization::Account::HashingParameters(parameters.ops_limit, parameters.mem_limit, parameters.algorithm)).Union();
    auto fbsRespAccount   = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder(), ThorQ::Serialization::Account::Body_hashing_parameters, fbsRespHashPrms).Union();
    context.addMessage(ThorQ::Serialization::Body_account, fbsRespAccount);
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageLoginRequest(ThorQ::HandlerContext& context)
{
    auto fbsLoginRequest = context.body<ThorQ::Serialization::Account::LoginRequest>();
    auto connection = context.apiConnection();

    if (fbsLoginRequest->account_id() == nullptr || fbsLoginRequest->account_id()->data() == nullptr || fbsLoginRequest->password_hash() == nullptr || fbsLoginRequest->password_hash()->hash() == nullptr) {
        connection->disconnect();
        return;
    }

    fmt::print("[ACCOUNT] LoginRequest\n");
    auto fbsAccountId = fbsLoginRequest->account_id()->data();
    auto fbsPasswordHash = fbsLoginRequest->password_hash()->hash();

    ThorQ::Uuid accountId(std::span<const std::uint8_t, 16>(fbsAccountId->data(), fbsAccountId->size()));
    ThorQ::Crypto::Hashing::Hash passwordHash;
    std::memcpy(passwordHash.data(), fbsPasswordHash->Data(), ThorQ::Crypto::Hashing::HashLength);

    auto account = ThorQ::Account::GetAccount(accountId);

    flatbuffers::Offset<void> fbsRespLogin;
    ThorQ::Serialization::Uuid fbsRespAccountID;
    ThorQ::Serialization::Account::AuthToken fbsRespAuthToken;
    if (account != nullptr && account->checkPasswordHash(passwordHash)) {
        fmt::print("[ACCOUNT] Login request ACCEPTED\n");
        connection->setAccount(account);

        fbsRespAccountID = ThorQ::Serialization::Uuid(account->uuid().toBytes());

        // TODO: Create auth token
        if (fbsLoginRequest->get_auth_token()) {
            // TODO: Get auth token
            fbsRespLogin = ThorQ::Serialization::Account::CreateLoginResponse(context.fbsBuilder(), true, &fbsRespAccountID, &fbsRespAuthToken).Union();
        }
        else {
            fbsRespLogin = ThorQ::Serialization::Account::CreateLoginResponse(context.fbsBuilder(), true, &fbsRespAccountID).Union();
        }
    }
    else {
        fmt::print("[ACCOUNT] Login request DENIED\n");
        fbsRespLogin = ThorQ::Serialization::Account::CreateLoginResponse(context.fbsBuilder(), false).Union();
    }

    auto fbsRespAccount = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder(), ThorQ::Serialization::Account::Body_login_response, fbsRespLogin).Union();
    context.addMessage(ThorQ::Serialization::Body_account, fbsRespAccount);
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageLogoutRequest(ThorQ::HandlerContext& context)
{
    auto fbsLogoutRequest = context.body<ThorQ::Serialization::Account::LogoutRequest>();
    auto connection = context.apiConnection();

    fmt::print("[ACCOUNT] LogoutRequest\n");

    bool success = false;
    auto account = connection->account();
    if (account != nullptr) {
        // TODO notify of logout

        if (fbsLogoutRequest->logout_all()) {
            // TODO remove all instances, and all session keys
        }

        success = true;
    }


    auto fbsRespLogout  = ThorQ::Serialization::Account::CreateLogoutResponse(context.fbsBuilder(), success).Union();
    auto fbsRespAccount = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder(), ThorQ::Serialization::Account::Body_logout_response, fbsRespLogout).Union();
    context.addMessage(ThorQ::Serialization::Body_account, fbsRespAccount);
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageRegistrationRequest(ThorQ::HandlerContext& context)
{
    auto fbsRegistrationReq = context.body<ThorQ::Serialization::Account::RegistrationRequest>();
    auto connection = context.apiConnection();

    if (fbsRegistrationReq->account_id() == nullptr || fbsRegistrationReq->account_id()->data() == nullptr || fbsRegistrationReq->email() == nullptr || fbsRegistrationReq->password_hash() == nullptr || fbsRegistrationReq->password_hash()->hash() == nullptr) {
        connection->disconnect();
        return;
    }

    fmt::print("[ACCOUNT] RegistrationRequest\n");
    auto fbsAccountId     = fbsRegistrationReq->account_id()->data();
    auto fbsEmail         = fbsRegistrationReq->email();
    auto fbsPhParent      = fbsRegistrationReq->password_hash();
    auto fbsPasswordHash  = fbsPhParent->hash();
    auto fbsPasswordSalt  = fbsPhParent->salt();
    auto fbsHashingOpsLim = fbsPhParent->ops_limit();
    auto fbsHashingMemLim = fbsPhParent->mem_limit();
    auto fbsHashingAlgo   = fbsPhParent->algorithm();

    ThorQ::Uuid accountID(std::span<const std::uint8_t, 16>(fbsAccountId->data(), fbsAccountId->size()));
    std::string email(fbsEmail->data(), fbsEmail->size());

    ThorQ::Crypto::Hashing::Hash passwordHash;
    std::memcpy(passwordHash.data(), fbsPasswordHash->Data(), ThorQ::Crypto::Hashing::HashLength);

    ThorQ::Crypto::Hashing::Salt passwordSalt;
    std::memcpy(passwordSalt.data(), fbsPasswordSalt->Data(), ThorQ::Crypto::Hashing::SaltLength);

    ThorQ::Crypto::Hashing::Parameters hashingParams;
    hashingParams.ops_limit = fbsHashingOpsLim;
    hashingParams.mem_limit = fbsHashingMemLim;
    hashingParams.algorithm = fbsHashingAlgo;

    fmt::print("[ACCOUNT] Client requested account: {}\n", accountID.toString());
    auto account = ThorQ::Account::GetAccount(accountID);

    if (account == nullptr) {
        throw MessageHandlingException("AccountID not found", 1); // TODO implement requestID's
    }

    if (!account->tryClaim(email, passwordHash, passwordSalt, hashingParams)) {
        fmt::print("[ACCOUNT] {} already taken!\n", account->username());
        // TODO respond with username/email taken
        return;
    }

    fmt::print("[ACCOUNT] {} claimed!\n", account->username());
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageRecoveryRequest(ThorQ::HandlerContext& context)
{
    auto fbsRecoveryReq = context.body<ThorQ::Serialization::Account::RecoveryRequest>();
    auto connection = context.apiConnection();

    if (fbsRecoveryReq->email() == nullptr) {
        connection->disconnect();
        return;
    }
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageDeletionRequest(ThorQ::HandlerContext& context)
{
    auto fbsDeletionReq = context.body<ThorQ::Serialization::Account::DeletionRequest>();
    auto connection = context.apiConnection();

    if (fbsDeletionReq->password_hash() == nullptr || fbsDeletionReq->password_hash()->hash() == nullptr) {
        connection->disconnect();
        return;
    }
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageUpdateRequest(ThorQ::HandlerContext& context)
{
    auto fbsUpdateReq = context.body<ThorQ::Serialization::Account::UpdateRequest>();
    auto connection = context.apiConnection();

    if (fbsUpdateReq->old_password() == nullptr || fbsUpdateReq->old_password()->hash() == nullptr) {
        connection->disconnect();
        return;
    }
}

void ThorQ::ApiEndpoints::AccountEndpoint::handleMessageSetImageRequest(ThorQ::HandlerContext& context)
{
    auto fbsUpdateReq = context.body<ThorQ::Serialization::Account::SetImageRequest>();
    auto connection = context.apiConnection();

    if (fbsUpdateReq->new_image() == nullptr) {
        connection->disconnect();
        return;
    }
}
