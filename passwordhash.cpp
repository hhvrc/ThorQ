#include "passwordhash.h"

#include <QThread>

#define SODIUM_STATIC
#include <sodium.h>

static_assert (PasswordHash::SaltLength == crypto_pwhash_SALTBYTES, "PasswordHashing salt length changed");
static_assert (PasswordHash::HashLength == crypto_box_SEEDBYTES, "PasswordHashing hash length changed");

PasswordHash::PasswordHash(QObject* parent)
    : QObject(parent)
    , m_salt()
    , m_hash()
    , m_opsLimit()
    , m_memLimit()
    , m_algorithm()
{
    m_salt.resize(PasswordHash::SaltLength);
    m_hash.resize(PasswordHash::HashLength);
}

PasswordHash::~PasswordHash()
{
}

void PasswordHash::generateSalt()
{
    randombytes_buf(m_salt.data(), m_salt.size());
    emit saltChanged();
}

void PasswordHash::setSalt(const QByteArray& salt)
{
    if (m_salt != salt) {
        m_salt = salt;
        emit saltChanged();
    }
}

void PasswordHash::setOpsLimit(quint64 opsLimit)
{
    if (m_opsLimit != opsLimit) {
        m_opsLimit = opsLimit;
        emit opsLimitChanged();
    }
}

void PasswordHash::setMemLimit(quint64 memLimit)
{
    if (m_memLimit != memLimit) {
        m_memLimit = memLimit;
        emit memLimitChanged();
    }
}

void PasswordHash::setAlgorithm(qint32 algorithm)
{
    if (m_algorithm != algorithm) {
        m_algorithm = algorithm;
        emit algorithmChanged();
    }
}

void PasswordHash::setPerformance(PasswordHash::PerformancePresets performance)
{
    switch (performance) {
    case PerformancePresets::Interactive:
        m_opsLimit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
        m_memLimit = crypto_pwhash_MEMLIMIT_INTERACTIVE;
        m_algorithm = crypto_pwhash_ALG_DEFAULT;
        break;
    case PerformancePresets::Moderate:
        m_opsLimit = crypto_pwhash_OPSLIMIT_MODERATE;
        m_memLimit = crypto_pwhash_MEMLIMIT_MODERATE;
        m_algorithm = crypto_pwhash_ALG_DEFAULT;
        break;
    case PerformancePresets::Sensitive:
        m_opsLimit = crypto_pwhash_OPSLIMIT_SENSITIVE;
        m_memLimit = crypto_pwhash_MEMLIMIT_SENSITIVE;
        m_algorithm = crypto_pwhash_ALG_DEFAULT;
        break;
    }
}

void PasswordHash::generateHash(const QString& password)
{
    HashingThread* worker = new HashingThread(password.toUtf8(), m_salt, m_opsLimit, m_memLimit, m_algorithm, this);
    connect(worker, &HashingThread::hashReady, this, &PasswordHash::handleWorkerDone);
    connect(worker, &HashingThread::hashFailed, this, &PasswordHash::handleWorkerFailed);
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

void PasswordHash::handleWorkerDone(const QByteArray& hash)
{
    if (m_hash != hash) {
        m_hash = hash;
        emit hashGenerated();
    }
}

void PasswordHash::handleWorkerFailed()
{
    emit hashFailed();
}

HashingThread::HashingThread(const QByteArray& passwordUtf8, const QByteArray& salt, uint64_t opsLimit, uint64_t memLimit, int32_t algorithm, QObject* parent)
    : QThread(parent)
    , m_passwordUtf8(passwordUtf8)
    , m_salt(salt)
    , m_opsLimit(opsLimit)
    , m_memLimit(memLimit)
    , m_algorithm(algorithm)
{
}

void HashingThread::run()
{
    QByteArray hash;
    hash.resize(PasswordHash::HashLength);

    if (crypto_pwhash(
                (std::uint8_t*)hash.data(), PasswordHash::HashLength,
                m_passwordUtf8.data(), m_passwordUtf8.length(),
                (std::uint8_t*)m_salt.data(),
                m_opsLimit,
                m_memLimit,
                m_algorithm
                ) == 0) {
        emit hashReady(hash);
    } else {
        emit hashFailed();
    }
}
