#ifndef AUTHTOKEN_H
#define AUTHTOKEN_H

#include <array>
#include <vector>
#include <cstdint>

#include <QString>
#include <QByteArray>
#include <QDateTime>

#include <crypto.h>
#include <systemid.h>

struct AuthToken
{
    QByteArray token; // 128-bit B64-Encoded authtoken
	QByteArray systemID;
	QDateTime createdAt;
};

inline AuthToken CreateAuthToken(const QByteArray& systemID)
{
    AuthToken authToken;

    authToken.token.resize(16);
    ThorQ::Crypto::RandomizeBytes((std::uint8_t*)authToken.token.data(), 16);

    authToken.systemID = systemID;
    authToken.createdAt = QDateTime::currentDateTimeUtc();

    return authToken;
}

#endif // AUTHTOKEN_H
