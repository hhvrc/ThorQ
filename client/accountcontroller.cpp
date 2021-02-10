#include "accountcontroller.h"

#include <schemas_common.h>

#include <fmt/core.h>

#include <QUuid>
#include <QDebug>

ThorQ::AccountController::AccountController(std::function<void(const std::span<std::uint8_t>&, bool)> onMessageGenerated, QObject *parent)
    : QObject(parent)
    , m_accountID()
    , m_authToken{0}
    , m_hashingParameters()
    , m_loggingIn(false)
    , m_username()
    , m_password()
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
    {
        auto fbsAccountId = reinterpret_cast<const ThorQ::Serialization::Uuid*>(message);
        m_accountID = ThorQ::Uuid(std::span<const std::uint8_t, 16>(fbsAccountId->data()->data(), 16));

        fmt::print("[ACCOUNT] Got systemID: {}\n", m_accountID.toString());
        if (m_loggingIn) {

        }
        break;
    }
    case ThorQ::Serialization::Account::Body_hashing_parameters:
    {
        auto fbsHashingParameters = *reinterpret_cast<const ThorQ::Serialization::Account::HashingParameters*>(message);
        memcpy(m_hashingParameters.salt.data(), fbsHashingParameters.salt()->data(), ThorQ::Crypto::Hashing::SaltLength);
        m_hashingParameters.ops_limit = fbsHashingParameters.ops_limit();
        m_hashingParameters.mem_limit = fbsHashingParameters.mem_limit();
        m_hashingParameters.algorithm = fbsHashingParameters.algorithm();
        requestAuthToken();
        break;
    }
    case ThorQ::Serialization::Account::Body_auth_token:
    {
        auto fbsAuthToken = reinterpret_cast<const ThorQ::Serialization::Account::AuthToken*>(message);
        memcpy(m_authToken.data(), fbsAuthToken->token()->data(), 64);
        break;
    }
    default:
        break;
    }
}

void ThorQ::AccountController::setUsername(const QString& username)
{
    qDebug() << username;
    m_username = username.toStdString();
}

void ThorQ::AccountController::setPassword(const QString& password)
{
    qDebug() << password;
    m_password = password.toStdString();
}

void ThorQ::AccountController::login()
{
    requestAccountId();
    m_loggingIn = true;

    qDebug() << "Login";
}

void ThorQ::AccountController::logout()
{
    qDebug() << "Logout";
}

void ThorQ::AccountController::requestAccountId()
{
    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsUsername = fbsBuilder.CreateString(m_username.data(), m_username.size());
    auto fbsGetID    = ThorQ::Serialization::Account::CreateGetAccountId(fbsBuilder, fbsUsername).Union();
    auto fbsAccount  = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_get_account_id, fbsGetID).Union();
    auto fbsMessage  = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);
    fbsBuilder.Finish(fbsMessage);

    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::AccountController::requestHashingParameters()
{
    ThorQ::Serialization::Uuid fbsAccountID(m_accountID.toBytes());

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsGetSeed = ThorQ::Serialization::Account::CreateGetHashingParameters(fbsBuilder, &fbsAccountID).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_get_hashing_parameters, fbsGetSeed).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);
    fbsBuilder.Finish(fbsMessage);

    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::AccountController::requestAuthToken()
{
    ThorQ::Serialization::Uuid fbsAccountID(m_accountID.toBytes());


    ThorQ::Serialization::Account::HashCalculated hash;
    if (!ThorQ::Crypto::Hashing::Generate(m_password, m_hashingParameters, hash.mutable_hash()->data())) {
        return;
    }

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsLogin   = ThorQ::Serialization::Account::CreateGetAuthToken(fbsBuilder, &fbsAccountID, &hash).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_get_auth_token, fbsLogin).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);
    fbsBuilder.Finish(fbsMessage);

    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::AccountController::requestLogin()
{
    ThorQ::Serialization::Uuid fbsAccountID(m_accountID.toBytes());

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsLogin   = ThorQ::Serialization::Account::CreateLogin(fbsBuilder, &fbsAccountID).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_login, fbsLogin).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);
    fbsBuilder.Finish(fbsMessage);

    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::AccountController::requestLogout()
{
    ThorQ::Serialization::Uuid fbsAccountID(m_accountID.toBytes());

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_logout).Union();
    auto fbsMessage = ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount);
    fbsBuilder.Finish(fbsMessage);

    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
