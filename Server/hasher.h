#ifndef HASHER_H
#define HASHER_H

#include <QObject>
#include <QRunnable>

class PasswordHasher : public QObject, public QRunnable
{
    Q_OBJECT
public:
    PasswordHasher(const QString& password, QObject* parent = nullptr);

    void run() override;
signals:
    void finished(const QString& hash);
private:
    QString m_password;
};
class PasswordVerifier : public QObject, public QRunnable
{
    Q_OBJECT
public:
    PasswordVerifier(const QString& hash, const QString& password, QObject* parent = nullptr);

    void run() override;
signals:
    void finished(bool matched);
private:
    QString m_hash;
    QString m_password;
};

#endif // HASHER_H
