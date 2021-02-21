#include "accountcontroller.h"

#include <schemas_common.h>

#include <fmt/core.h>

#include <QUuid>
#include <QDebug>

ThorQ::AccountController::AccountController(std::function<void(const std::span<std::uint8_t>&, bool)> onMessageGenerated, QObject *parent)
    : QObject(parent)
    , m_activeUser(nullptr)
    , m_email()
    , m_temporaryPassword()
    , m_authToken{0}
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
    {
        auto fbsAccountId = reinterpret_cast<const ThorQ::Serialization::Uuid*>(fbsAccount->body())->data();
        std::span<const std::uint8_t, 16> uuidBytes(
                    fbsAccountId->data(),
                    fbsAccountId->size()
                    );

        m_activeUser = new ThorQ::User(ThorQ::Uuid(uuidBytes), this);

        fmt::print("[ACCOUNT] Got accountID: {}\n", m_activeUser->id().toString());
        if (!m_temporaryPassword.isEmpty()) {
            requestHashingParameters();
        }
        break;
    }
    case ThorQ::Serialization::Account::Body_hashing_parameters:
    {
        auto fbsHashingParameters = *reinterpret_cast<const ThorQ::Serialization::Account::HashingParameters*>(fbsAccount->body());
        memcpy(m_hashingParameters.salt.data(), fbsHashingParameters.salt()->data(), ThorQ::Crypto::Hashing::SaltLength);
        m_hashingParameters.ops_limit = fbsHashingParameters.ops_limit();
        m_hashingParameters.mem_limit = fbsHashingParameters.mem_limit();
        m_hashingParameters.algorithm = fbsHashingParameters.algorithm();
        fmt::print("[ACCOUNT] Got HashingParameters: {} {} {}\n", m_hashingParameters.ops_limit, m_hashingParameters.mem_limit, m_hashingParameters.algorithm);
        requestLogin(true);
        break;
    }
    case ThorQ::Serialization::Account::Body_login_response:
    {
        auto fbsLoginResponse = reinterpret_cast<const ThorQ::Serialization::Account::LoginResponse*>(fbsAccount->body());

        if (fbsLoginResponse->success()) {
            fmt::print("[ACCOUNT] Logged in!\n");
            if (fbsLoginResponse->auth_token() != nullptr) {
                memcpy(m_authToken.data(), fbsLoginResponse->auth_token()->token()->data(), 64);
            }
        }
        else {
            fmt::print("[ACCOUNT] Inconnect username/password!\n");
        }
        break;
    }
    default:
        break;
    }
}

void ThorQ::AccountController::login(const QString& username, const QString& password)
{
    m_temporaryPassword = password;
    requestAccountId(username);
    qDebug() << "Login:" << username << password;
}

void ThorQ::AccountController::logout()
{
    qDebug() << "Logout";
}

void ThorQ::AccountController::requestAccountId(const QString& username)
{
    auto stdstr = username.toStdString();

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsUsername = fbsBuilder.CreateString(stdstr.data(), stdstr.size());
    auto fbsGetID    = ThorQ::Serialization::Account::CreateGetAccountId(fbsBuilder, fbsUsername).Union();
    auto fbsAccount  = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_get_account_id, fbsGetID).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::AccountController::requestHashingParameters()
{
    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsGetSeed = ThorQ::Serialization::Account::CreateGetHashingParameters(fbsBuilder, &fbsAccountID).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_get_hashing_parameters, fbsGetSeed).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::AccountController::requestLogin(bool getAuthToken)
{
    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    ThorQ::Serialization::Account::HashCalculated hash;
    if (!ThorQ::Crypto::Hashing::Generate(m_temporaryPassword.toStdString(), m_hashingParameters, hash.mutable_hash()->data())) {
        return;
    }
    m_temporaryPassword.clear();

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsLogin   = ThorQ::Serialization::Account::CreateLoginRequest(fbsBuilder, &fbsAccountID, &hash, getAuthToken).Union();
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_login_request, fbsLogin).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}

void ThorQ::AccountController::requestLogout()
{
    ThorQ::Serialization::Uuid fbsAccountID(m_activeUser->id().toBytes());

    flatbuffers::FlatBufferBuilder fbsBuilder;
    auto fbsAccount = ThorQ::Serialization::Account::CreateMessage(fbsBuilder, ThorQ::Serialization::Account::Body_logout).Union();

    std::vector<flatbuffers::Offset<ThorQ::Serialization::Message>> messages;
    messages.push_back(ThorQ::Serialization::CreateMessage(fbsBuilder, ThorQ::Serialization::Body_account, fbsAccount));
    fbsBuilder.Finish(ThorQ::Serialization::CreateMessageBufferDirect(fbsBuilder, &messages));
    f_encodeAndSend(fbsBuilder.GetBufferSpan(), true);
}
