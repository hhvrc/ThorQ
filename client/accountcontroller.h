#ifndef ACCOUNTCONTROLLER_H
#define ACCOUNTCONTROLLER_H

#include <uuid.h>
#include <schemas_common.h>
#include <cryptography/hashing.h>

#include <flatbuffers/flatbuffers.h>

#include <QUuid>
#include <QByteArray>
#include <QString>
#include <QObject>
#include <QSharedPointer>

#include <array>
#include <string>

namespace ThorQ {
class AccountController : public QObject
{
    Q_OBJECT
public:
    AccountController(QObject *parent = nullptr);

signals:
    void messageGenerated(const QByteArray& buffer);
public slots:
    void ParseMessage(const void* message);
    void setUsername(const QString& username);
    void setPassword(const QString& password);
    void login();
    void logout();
private:
    void requestAccountId();
    void requestHashingParameters();
    void requestAuthToken();
    void requestLogin();
    void requestLogout();

    ThorQ::Uuid m_accountID;
    std::array<std::uint8_t, 64> m_authToken;
    ThorQ::Crypto::Hashing::HashingParameters m_hashingParameters;

    bool m_loggingIn;
    std::string m_username;
    std::string m_password;
};
}

#endif // ACCOUNTCONTROLLER_H
