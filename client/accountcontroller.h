#ifndef LOGINCONTROLLER_H
#define LOGINCONTROLLER_H

#include <QObject>
#include <QSharedPointer>

namespace ThorQ {
class AccountController : public QObject
{
    Q_OBJECT
public:
    AccountController(QObject *parent = nullptr);

signals:
public slots:
    void setUsername(const QString& username);
    void setPassword(const QString& password);
    void login();
    void logout();
private:
    QString m_username;
    QString m_password;
};
}

#endif // LOGINCONTROLLER_H
