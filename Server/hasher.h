#ifndef HASHER_H
#define HASHER_H

#include <QThread>

class PasswordHasher : public QThread
{
    Q_OBJECT
public:
    PasswordHasher(const QString& password, QObject* parent = nullptr);
signals:
    void hashingDone(const QString& hash);
private:
    void run() override;

    QString m_password;
};
class PasswordVerifier : public QThread
{
    Q_OBJECT
public:
    PasswordVerifier(const QString& hash, const QString& password, QObject* parent = nullptr);
signals:
    void verificationDone(bool matched);
private:
    void run() override;

    QString m_hash;
    QString m_password;
};

#endif // HASHER_H
