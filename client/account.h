#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "user.h"

#include <QObject>
#include <QUuid>
#include <QString>
#include <QByteArray>

namespace ThorQ {
class Account : public ThorQ::User
{
    Q_OBJECT
    Q_DISABLE_COPY(Account)
public:
    Account(QUuid userID, QObject* parent = nullptr);
private:
    QUuid m_id;
    QString m_email;
    QByteArray m_passwordHash;
};
}

#endif // ACCOUNT_H
