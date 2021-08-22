#ifndef PASSWORDHASH_H
#define PASSWORDHASH_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QThread>

#include <cstdint>

class PasswordHash : public QObject
{
    Q_OBJECT
public:
    static const std::size_t SaltLength = 16;
    static const std::size_t HashLength = 32;

    enum class PerformancePresets {
        Interactive,
        Moderate,
        Sensitive
    };
    Q_ENUMS(PerformancePresets)

    PasswordHash(QObject* parent = nullptr);
    ~PasswordHash();

    QByteArray salt() const;
    QByteArray hash() const;
    quint64 opsLimit() const;
    quint64 memLimit() const;
    qint32 algorithm() const;
signals:
    void saltChanged();
    void opsLimitChanged();
    void memLimitChanged();
    void algorithmChanged();

    void hashGenerated();
    void hashFailed();
public slots:
    void generateSalt();
    void setSalt(const QByteArray& salt);

    void setOpsLimit(quint64 opsLimit);
    void setMemLimit(quint64 memLimit);
    void setAlgorithm(qint32 algorithm);
    void setPerformance(PerformancePresets performance);

    void generateHash(const QString& password);
private slots:
    void handleWorkerDone(const QByteArray& hash);
    void handleWorkerFailed();
private:
    QByteArray m_salt;
    QByteArray m_hash;
    std::uint64_t m_opsLimit;
    std::uint64_t m_memLimit;
    std::int32_t  m_algorithm;
};
class HashingThread : public QThread {
    Q_OBJECT
public:
    HashingThread(const QByteArray& passwordUtf8, const QByteArray& salt, std::uint64_t opsLimit, std::uint64_t memLimit, std::int32_t algorithm, QObject* parent = nullptr);
signals:
    void hashReady(const QByteArray& hash);
    void hashFailed();
private:
    void run() override;

    QByteArray m_passwordUtf8;
    QByteArray m_salt;
    std::uint64_t m_opsLimit;
    std::uint64_t m_memLimit;
    std::int32_t  m_algorithm;
};

#endif // PASSWORDHASH_H
