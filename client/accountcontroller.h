#ifndef ACCOUNTCONTROLLER_H
#define ACCOUNTCONTROLLER_H

#include "user.h"

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
    AccountController(std::function<void(const std::span<std::uint8_t>&, bool)> encodeAndSend, QObject* parent = nullptr);

    QString email();
signals:
    void loggedIn();
    void loggedOut();
public slots:
    void ParseMessage(const void* message);
    void login(const QString& username, const QString& password);
    void logout();
    void registerAccount(const QString& username, const QString& email, const QString& password);
    void recoverAccount(const QString& email);
private:
    void handleMessageAccountId(const void* body);
    void handleMessageHashingSalt(const void* body);
    void handleMessageHashingParameters(const void* body);
    void handleMessageLoginResponse(const void* body);
    void handleMessageLogoutResponse(const void* body);
    void handleMessageRegistrationResponse(const void* body);

    void requestAccountId();
    void requestHashingSalt(bool newPassword);
    void requestHashingParameters();
    void requestLogin(bool getAuthToken);
    void requestLogout(bool logoutAll);
    void requestRegistration();
    void requestRecovery();

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

    std::function<void(const std::span<std::uint8_t>&, bool)> f_encodeAndSend;
};
}

#endif // ACCOUNTCONTROLLER_H
