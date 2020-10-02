#include "hasher.h"

#include <botan_all.h>

PasswordHasher::PasswordHasher(const QString& password, QObject* parent)
    : QThread(parent)
    , m_password(password)
{
}

void PasswordHasher::run()
{
    Botan::AutoSeeded_RNG rng = Botan::AutoSeeded_RNG();

    std::size_t oldSize = m_password.size();
    m_password = QString::fromStdString(Botan::generate_bcrypt(m_password.toStdString(), rng));
    std::size_t newSize = m_password.size();

    if (newSize < oldSize)
    {
        m_password.resize(oldSize);
        memset(m_password.data() + newSize, 0, newSize - oldSize);
        m_password.resize(newSize);
    }

    emit hashingDone(m_password);
}

PasswordVerifier::PasswordVerifier(const QString& hash, const QString& password, QObject* parent)
    : QThread(parent)
    , m_hash(hash)
    , m_password(password)
{
}

void PasswordVerifier::run()
{
    bool result = Botan::check_bcrypt(m_password.toStdString(), m_hash.toStdString());
    memset(m_password.data(), 0, m_password.size());
    emit verificationDone(result);
}
