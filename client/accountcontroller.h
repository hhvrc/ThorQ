#ifndef ACCOUNTCONTROLLER_H
#define ACCOUNTCONTROLLER_H

#include "user.h"
#include "apiclient.h"

#include <uuid.h>
#include <cryptography/hashing.h>

#include <QUuid>
#include <QString>
#include <QObject>

#include <functional>
#include <span>
#include <array>
#include <string>
#include <cstdint>

namespace ThorQ {
class AccountController : public QObject
{
    Q_OBJECT
public:
    AccountController(std::function<bool(MessageContext&)> sendContextData, QObject* parent = nullptr);

    QString email();

    void ParseMessage(MessageContext& context);
signals:
    void loggedIn();
    void loggedOut();
public slots:
    void login(const QString& username, const QString& password);
    void logout();
    void registerAccount(const QString& username, const QString& email, const QString& password);
    void recoverAccount(const QString& email);
private:
    void handleMessageAccountId(MessageContext& context);
    void handleMessageHashingSalt(MessageContext& context);
    void handleMessageHashingParameters(MessageContext& context);
    void handleMessageLoginResponse(MessageContext& context);
    void handleMessageLogoutResponse(MessageContext& context);
    void handleMessageRegistrationResponse(MessageContext& context);

    void requestAccountId(MessageContext& context);
    void requestHashingSalt(bool newPassword, MessageContext& context);
    void requestHashingParameters(MessageContext& context);
    void requestLogin(bool getAuthToken, MessageContext& context);
    void requestLogout(bool logoutAll, MessageContext& context);
    void requestRegistration(MessageContext& context);
    void requestRecovery(MessageContext& context);

    ThorQ::User* m_activeUser;

    enum class LastRequest {
        None,
        Login,
        Logout,
        Register,
        Recover
    } m_lastRequest;
    std::string m_requestUsername;
    std::string m_requestEmail;
    std::string m_requestPassword;

    std::array<std::uint8_t, 64> m_authToken;

    bool m_gotHashingSalt;
    ThorQ::Crypto::Hashing::Salt m_hashingSalt;

    bool m_gotHashingParameters;
    ThorQ::Crypto::Hashing::Parameters m_hashingParameters;

    std::function<bool(MessageContext&)> f_sendContextData;
};
}

#endif // ACCOUNTCONTROLLER_H
