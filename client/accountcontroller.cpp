#include "accountcontroller.h"

#include <schemas_common.h>

#include <fmt/core.h>

#include <QUuid>
#include <QDebug>

ThorQ::AccountController::AccountController(std::function<bool(HandlerContext&)> sendContextData, QObject *parent)
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
    , f_sendContextData(sendContextData)
{
}

void ThorQ::AccountController::ParseMessage(HandlerContext& context)
{
    auto fbsAccount = reinterpret_cast<const ThorQ::Serialization::Account::Message*>(context.body);

    fmt::print("[MSG] Account\n");
    context.body = fbsAccount->body();
    if (context.body == nullptr) {
        return;
    }

    switch (fbsAccount->body_type())
    {
    case ThorQ::Serialization::Account::Body_account_id:
        handleMessageAccountId(context);
        break;
    case ThorQ::Serialization::Account::Body_hashing_salt:
        handleMessageHashingSalt(context);
        break;
    case ThorQ::Serialization::Account::Body_hashing_parameters:
        handleMessageHashingParameters(context);
        break;
    case ThorQ::Serialization::Account::Body_login_response:
        handleMessageLoginResponse(context);
        break;
    case ThorQ::Serialization::Account::Body_logout_response:
        handleMessageLogoutResponse(context);
        break;
    case ThorQ::Serialization::Account::Body_registration_response:
        handleMessageRegistrationResponse(context);
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

    HandlerContext context;
    requestAccountId(context);
    f_sendContextData(context);
}

void ThorQ::AccountController::logout()
{
    m_requestUsername.clear();
    m_requestEmail.clear();
    m_requestPassword.clear();

    m_lastRequest = LastRequest::Logout;

    HandlerContext context;
    requestLogout(false, context);
    f_sendContextData(context);

    qDebug() << "Logout";
}

void ThorQ::AccountController::registerAccount(const QString& username, const QString& email, const QString& password)
{
    m_requestUsername = username.toStdString();
    m_requestEmail = email.toStdString();
    m_requestPassword = password.toStdString();

    m_lastRequest = LastRequest::Register;

    HandlerContext context;
    requestAccountId(context);
    f_sendContextData(context);
}

void ThorQ::AccountController::recoverAccount(const QString& email)
{
    m_requestUsername.clear();
    m_requestEmail = email.toStdString();
    m_requestPassword.clear();

    m_lastRequest = LastRequest::Recover;

    HandlerContext context;
    requestRecovery(context);
    f_sendContextData(context);
}

void ThorQ::AccountController::handleMessageAccountId(HandlerContext& context)
{
    auto fbsAccountId = reinterpret_cast<const ThorQ::Serialization::Uuid*>(context.body)->data();
    std::span<const std::uint8_t, 16> uuidBytes(
                fbsAccountId->data(),
                fbsAccountId->size()
                );

    m_activeUser = new ThorQ::User(ThorQ::Uuid(uuidBytes), this);

    fmt::print("[ACCOUNT] Got accountID: {}\n", m_activeUser->id().toString());

    switch (m_lastRequest) {
    case LastRequest::Login:
        requestHashingSalt(false, context);
        requestHashingParameters(context);
        return;
    case LastRequest::Register:
        requestHashingSalt(true, context);
        requestHashingParameters(context);
        return;
    default:
        break;
    }

    m_requestUsername.clear();
    m_requestEmail.clear();
    m_requestPassword.clear();
}

void ThorQ::AccountController::handleMessageHashingSalt(HandlerContext& context)
{
    auto fbsHashingSalt = reinterpret_cast<const ThorQ::Serialization::Account::HashingSalt*>(context.body)->salt();

    memcpy(m_hashingSalt.data(), fbsHashingSalt->data(), ThorQ::Crypto::Hashing::SaltLength);
    m_gotHashingSalt = true;

    fmt::print("[ACCOUNT] Got HashingSalt!\n");

    switch (m_lastRequest) {
    case LastRequest::Login:
        if (m_gotHashingParameters) {
            requestLogin(true, context);
        }
        return;
    case LastRequest::Register:
        if (m_gotHashingParameters) {
            requestRegistration(context);
        }
        return;
    default:
        break;
    }

    m_requestUsername.clear();
    m_requestEmail.clear();
    m_requestPassword.clear();
}

void ThorQ::AccountController::handleMessageHashingParameters(HandlerContext& context)
{
    auto fbsHashingParameters = reinterpret_cast<const ThorQ::Serialization::Account::HashingParameters*>(context.body);

    m_hashingParameters.ops_limit = fbsHashingParameters->ops_limit();
    m_hashingParameters.mem_limit = fbsHashingParameters->mem_limit();
    m_hashingParameters.algorithm = fbsHashingParameters->algorithm();
    m_gotHashingParameters = true;

    fmt::print("[ACCOUNT] Got HashingParameters\n");

    switch (m_lastRequest) {
    case LastRequest::Login:
        if (m_gotHashingSalt) {
            requestLogin(true, context);
        }
        return;
    case LastRequest::Register:
        if (m_gotHashingSalt) {
            requestRegistration(context);
        }
        return;
    default:
        break;
    }

    m_requestUsername.clear();
    m_requestEmail.clear();
    m_requestPassword.clear();
}

void ThorQ::AccountController::handleMessageLoginResponse(HandlerContext& context)
{
    auto fbsLoginResponse = reinterpret_cast<const ThorQ::Serialization::Account::LoginResponse*>(context.body);

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

void ThorQ::AccountController::handleMessageLogoutResponse(HandlerContext& context)
{
    auto fbsLogoutResponse = reinterpret_cast<const ThorQ::Serialization::Account::LogoutResponse*>(context.body);

    if (fbsLogoutResponse->success()) {
        fmt::print("[ACCOUNT] Logged out!\n");
        emit loggedOut();
    }
    else {
        fmt::print("[ACCOUNT] Error logging out!\n");
    }
}

void ThorQ::AccountController::handleMessageRegistrationResponse(HandlerContext& context)
{
    auto fbsRegistrationResponse = reinterpret_cast<const ThorQ::Serialization::Account::RegistrationResponse*>(context.body);

    if (fbsRegistrationResponse->success()) {
        fmt::print("[ACCOUNT] Account created!\n");
    }
    else {
        fmt::print("[ACCOUNT] Username/Email already used!\n");
    }
}

void ThorQ::AccountController::requestAccountId(HandlerContext& context)
{
    fmt::print("[ACCOUNT] requestAccountId()\n");

    auto fbsUsername = context.fbsBuilder.CreateString(m_requestUsername);
    auto fbsGetID    = ThorQ::Serialization::Account::CreateGetAccountId(context.fbsBuilder, fbsUsername).Union();
    auto fbsAccount  = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_get_account_id, fbsGetID).Union();
    auto fbsMessage  = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);

    context.messages.push_back(fbsMessage);
}
void ThorQ::AccountController::requestHashingSalt(bool newPassword, HandlerContext& context)
{
    fmt::print("[ACCOUNT] requestHashingSalt()\n");

    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    auto fbsGetSalt = ThorQ::Serialization::Account::CreateGetHashingSalt(context.fbsBuilder, &fbsAccountID, newPassword).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_get_hashing_salt, fbsGetSalt).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);

    context.messages.push_back(fbsMessage);
}
void ThorQ::AccountController::requestHashingParameters(HandlerContext& context)
{
    fmt::print("[ACCOUNT] requestHashingParameters()\n");

    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    auto fbsGetParams = ThorQ::Serialization::Account::CreateGetHashingParameters(context.fbsBuilder, &fbsAccountID).Union();
    auto fbsAccount   = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_get_hashing_parameters, fbsGetParams).Union();
    auto fbsMessage   = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);

    context.messages.push_back(fbsMessage);
}
void ThorQ::AccountController::requestLogin(bool getAuthToken, HandlerContext& context)
{
    fmt::print("[ACCOUNT] requestLogin()\n");

    m_gotHashingSalt = false;
    m_gotHashingParameters = false;

    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    ThorQ::Serialization::Account::HashCalculated hash;
    if (!ThorQ::Crypto::Hashing::Generate(m_requestPassword, m_hashingSalt, m_hashingParameters, hash.mutable_hash()->data())) {
        return;
    }
    m_requestPassword.clear();

    auto fbsLogin   = ThorQ::Serialization::Account::CreateLoginRequest(context.fbsBuilder, &fbsAccountID, &hash, getAuthToken).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_login_request, fbsLogin).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);

    context.messages.push_back(fbsMessage);
}
void ThorQ::AccountController::requestLogout(bool logoutAll, HandlerContext& context)
{
    fmt::print("[ACCOUNT] requestLogout()\n");

    m_gotHashingSalt = false;
    m_gotHashingParameters = false;

    auto fbsLogout  = ThorQ::Serialization::Account::CreateLogoutRequest(context.fbsBuilder, logoutAll).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_logout_request, fbsLogout).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);

    context.messages.push_back(fbsMessage);
}
void ThorQ::AccountController::requestRegistration(HandlerContext& context)
{
    fmt::print("[ACCOUNT] requestRegistration()\n");

    m_gotHashingSalt = false;
    m_gotHashingParameters = false;

    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    // TODO: make this adjustable from UI
    m_hashingParameters.setPerformance(ThorQ::Crypto::Hashing::Parameters::Performance::Sensitive);

    ThorQ::Crypto::Hashing::Hash calculatedHash;
    if (!ThorQ::Crypto::Hashing::Generate(m_requestPassword, m_hashingSalt, m_hashingParameters, calculatedHash)) {
        return;
    }
    m_requestPassword.clear();

    ThorQ::Serialization::Account::HashNew fbsHash(calculatedHash, m_hashingSalt, m_hashingParameters.ops_limit, m_hashingParameters.mem_limit, m_hashingParameters.algorithm);

    auto fbsEmail    = context.fbsBuilder.CreateString(m_requestEmail);
    auto fbsRegister = ThorQ::Serialization::Account::CreateRegistrationRequest(context.fbsBuilder, &fbsAccountID, fbsEmail, &fbsHash).Union();
    auto fbsAccount  = ThorQ::Serialization::Account::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Account::Body_registration_request, fbsRegister).Union();
    auto fbsMessage  = ThorQ::Serialization::CreateMessage(context.fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);

    context.messages.push_back(fbsMessage);
}

void ThorQ::AccountController::requestRecovery(HandlerContext& context)
{
    fmt::print("[ACCOUNT] requestRecovery()\n");
}
