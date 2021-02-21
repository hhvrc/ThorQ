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
public slots:
    void ParseMessage(const void* message);
    void login(const QString& username, const QString& password);
    void logout();
private:
    void requestAccountId(const QString& username);
    void requestHashingParameters();
    void requestLogin(bool getAuthToken);
    void requestLogout();

    ThorQ::User* m_activeUser;

    QString m_email;
    QString m_temporaryPassword;

    std::array<std::uint8_t, 64> m_authToken;

    ThorQ::Crypto::Hashing::HashingParameters m_hashingParameters;

    std::function<void(const std::span<std::uint8_t>&, bool)> f_encodeAndSend;
};
}

#endif // ACCOUNTCONTROLLER_H
