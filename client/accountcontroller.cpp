#include "accountcontroller.h"

#include <schemas_common.h>

#include <fmt/core.h>

#include <QUuid>
#include <QDebug>

ThorQ::AccountController::AccountController(std::function<void(const std::span<std::uint8_t>&, bool)> onMessageGenerated, QObject *parent)
    : QObject(parent)
    , m_activeUser(nullptr)
    , m_lastRequest(LastRequest::None)
    , m_requestEmail()
    , m_requestPassword()
    , m_authToken{0}
    , m_gotHashingSalt(false)
    , m_hashingSalt()
    , m_gotHashingParameters(false)
    , m_hashingParameters()
    , f_encodeAndSend(onMessageGenerated)
{
}

void ThorQ::AccountController::ParseMessage(const void* message)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::Account::Message*>(message);

    fmt::print("[MSG] Account\n");

    switch (fbsAccount->body_type())
    {
    case ThorQ::Serialization::Account::Body_account_id:
        handleMessageAccountId(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_hashing_salt:
        handleMessageHashingSalt(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_hashing_parameters:
        handleMessageHashingParameters(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_login_response:
        handleMessageLoginResponse(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_logout_response:
        handleMessageLogoutResponse(fbsAccount->body());
        break;
    case ThorQ::Serialization::Account::Body_registration_response:
        handleMessageRegistrationResponse(fbsAccount->body());
        break;
    default:
        break;
    }
}

void ThorQ::AccountController::login(const QString& username, const QString& password)
{
    m_requestUsername = username.toStdString();
    m_requestEmail.clear();
    m_requestPassword = password.toStdString();

    m_lastRequest = LastRequest::Login;

    requestAccountId();
}

void ThorQ::AccountController::logout()
{
    m_requestUsername.clear();
    m_requestEmail.clear();
    m_requestPassword.clear();

    m_lastRequest = LastRequest::Logout;

    requestLogout(false);

    qDebug() << "Logout";
}

void ThorQ::AccountController::registerAccount(const QString& username, const QString& email, const QString& password)
{
    m_requestUsername = username.toStdString();
    m_requestEmail = email.toStdString();
    m_requestPassword = password.toStdString();

    m_lastRequest = LastRequest::Register;

    requestAccountId();
}

void ThorQ::AccountController::recoverAccount(const QString& email)
{
    m_requestUsername.clear();
    m_requestEmail = email.toStdString();
    m_requestPassword.clear();

    m_lastRequest = LastRequest::Recover;

    requestRecovery();
}

void ThorQ::AccountController::handleMessageAccountId(const void* body)
{
    auto fbsAccountId = reinterpret_cast<const ThorQ::Serialization::Uuid*>(body)->data();
    std::span<const std::uint8_t, 16> uuidBytes(
                fbsAccountId->data(),
                fbsAccountId->size()
                );

    m_activeUser = new ThorQ::User(ThorQ::Uuid(uuidBytes), this);

    fmt::print("[ACCOUNT] Got accountID: {}\n", m_activeUser->id().toString());

    switch (m_lastRequest) {
    case LastRequest::Login:
        requestHashingSalt();
        requestHashingParameters();
        return;
    case LastRequest::Register:
        requestRegistration();
        return;
    default:
        break;
    }

    m_requestUsername.clear();
    m_requestEmail.clear();
    m_requestPassword.clear();
}

void ThorQ::AccountController::handleMessageHashingSalt(const void* body)
{
    auto fbsHashingSalt = reinterpret_cast<const ThorQ::Serialization::Account::HashingSalt*>(body)->salt();

    memcpy(m_hashingSalt.data(), fbsHashingSalt->data(), ThorQ::Crypto::Hashing::SaltLength);
    m_gotHashingSalt = true;

    fmt::print("[ACCOUNT] Got HashingSalt!\n");

    switch (m_lastRequest) {
    case LastRequest::Login:
        if (m_gotHashingParameters) {
            requestLogin(true);
        }
        return;
    case LastRequest::Register:
        requestRegistration();
        return;
    default:
        break;
    }

    m_requestUsername.clear();
    m_requestEmail.clear();
    m_requestPassword.clear();
}

void ThorQ::AccountController::handleMessageHashingParameters(const void* body)
{
    auto fbsHashingParameters = *reinterpret_cast<const ThorQ::Serialization::Account::HashingParameters*>(body);

    m_hashingParameters.ops_limit = fbsHashingParameters.ops_limit();
    m_hashingParameters.mem_limit = fbsHashingParameters.mem_limit();
    m_hashingParameters.algorithm = fbsHashingParameters.algorithm();
    m_gotHashingParameters = true;

    fmt::print("[ACCOUNT] Got HashingParameters\n");

    switch (m_lastRequest) {
    case LastRequest::Login:
        if (m_gotHashingSalt) {
            requestLogin(true);
        }
        return;
    default:
        break;
    }

    m_requestUsername.clear();
    m_requestEmail.clear();
    m_requestPassword.clear();
}

void ThorQ::AccountController::handleMessageLoginResponse(const void* body)
{
    auto fbsLoginResponse = reinterpret_cast<const ThorQ::Serialization::Account::LoginResponse*>(body);

    if (fbsLoginResponse->success()) {
        fmt::print("[ACCOUNT] Logged in!\n");
        if (fbsLoginResponse->auth_token() != nullptr) {
            memcpy(m_authToken.data(), fbsLoginResponse->auth_token()->token()->data(), 64);
        }
        emit loggedIn();
    }
    else {
        fmt::print("[ACCOUNT] Inconnect username/password!\n");
    }
}

void ThorQ::AccountController::handleMessageLogoutResponse(const void* body)
{
    auto fbsLogoutResponse = reinterpret_cast<const ThorQ::Serialization::Account::LogoutResponse*>(body);

    if (fbsLogoutResponse->success()) {
        fmt::print("[ACCOUNT] Logged out!\n");
        emit loggedOut();
    }
    else {
        fmt::print("[ACCOUNT] Error logging out!\n");
    }
}

void ThorQ::AccountController::handleMessageRegistrationResponse(const void* body)
{
    auto fbsRegistrationResponse = reinterpret_cast<const ThorQ::Serialization::Account::RegistrationResponse*>(body);

    if (fbsRegistrationResponse->success()) {
        fmt::print("[ACCOUNT] Account created!\n");
    }
    else {
        fmt::print("[ACCOUNT] Username/Email already used!\n");
    }
}

void ThorQ::AccountController::requestAccountId()
{
    fmt::print("[ACCOUNT] requestAccountId()\n");

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsUsername = fbsBuilder.CreateString(m_requestUsername);
    auto fbsGetID    = ThorQ::Serialization::Account::CreateGetAccountId(fbsBuilder, fbsUsername).Union();
    auto fbsAccount  = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_get_account_id, fbsGetID).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
void ThorQ::AccountController::requestHashingSalt()
{
    fmt::print("[ACCOUNT] requestHashingSalt()\n");

    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsGetSalt = ThorQ::Serialization::Account::CreateGetHashingSalt(fbsBuilder, &fbsAccountID).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_get_hashing_salt, fbsGetSalt).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
void ThorQ::AccountController::requestHashingParameters()
{
    fmt::print("[ACCOUNT] requestHashingParameters()\n");

    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsGetParams = ThorQ::Serialization::Account::CreateGetHashingParameters(fbsBuilder, &fbsAccountID).Union();
    auto fbsAccount   = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_get_hashing_parameters, fbsGetParams).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
void ThorQ::AccountController::requestLogin(bool getAuthToken)
{
    fmt::print("[ACCOUNT] requestLogin()\n");

    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    ThorQ::Serialization::Account::HashCalculated hash;
    if (!ThorQ::Crypto::Hashing::Generate(m_requestPassword, m_hashingSalt, m_hashingParameters, hash.mutable_hash()->data())) {
        return;
    }
    m_requestPassword.clear();

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsLogin   = ThorQ::Serialization::Account::CreateLoginRequest(fbsBuilder, &fbsAccountID, &hash, getAuthToken).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_login_request, fbsLogin).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
void ThorQ::AccountController::requestLogout(bool logoutAll)
{
    fmt::print("[ACCOUNT] requestLogout()\n");

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsLogout  = ThorQ::Serialization::Account::CreateLogoutRequest(fbsBuilder, logoutAll).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_logout_request, fbsLogout).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
void ThorQ::AccountController::requestRegistration()
{
    fmt::print("[ACCOUNT] requestRegistration()\n");

    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    // TODO: make this adjustable from UI
    m_hashingParameters.setPerformance(ThorQ::Crypto::Hashing::Parameters::Performance::Sensitive);

    ThorQ::Crypto::Hashing::Hash calculatedHash;
    if (!ThorQ::Crypto::Hashing::Generate(m_requestPassword, m_hashingSalt, m_hashingParameters, calculatedHash)) {
        return;
    }
    m_requestPassword.clear();

    ThorQ::Serialization::Account::HashingParameters fbsParams(m_hashingParameters.ops_limit, m_hashingParameters.mem_limit, m_hashingParameters.algorithm);
    ThorQ::Serialization::Account::HashNew fbsHash(calculatedHash, m_hashingSalt, fbsParams);

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsEmail    = fbsBuilder.CreateString(m_requestEmail);
    auto fbsRegister = ThorQ::Serialization::Account::CreateRegistrationRequest(fbsBuilder, &fbsAccountID, fbsEmail, &fbsHash).Union();
    auto fbsAccount  = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_registration_request, fbsRegister).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::AccountController::requestRecovery()
{
    fmt::print("[ACCOUNT] requestRecovery()\n");
}
