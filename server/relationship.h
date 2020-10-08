#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include <atomic>

#include <QObject>
#include <QUuid>

#include <enums.h>
#include "typedefs_server.h"

namespace ThorQ {
class Relationship : public QObject
{
    Q_OBJECT
public:
    Relationship(int privateId, QUuid publicId, Account* source, Account* target, THORQ_RELATIONSHIP_STATUS status, THORQ_RELATIONSHIP_AUTHORITY authority, QObject* parent = nullptr)
        : QObject(parent)
        , publicId(publicId)
        , m_privId(privateId)
        , m_source(source)
        , m_target(target)
        , m_status(status)
        , m_authority(authority)
    {
    }

    const QUuid publicId;

    Account* source() const;
    Account* target() const;

    THORQ_RELATIONSHIP_STATUS status() const;
    THORQ_RELATIONSHIP_AUTHORITY authority() const;
signals:
    void statusChanged(THORQ_RELATIONSHIP_STATUS status);
    void authorityChanged(THORQ_RELATIONSHIP_AUTHORITY authority);
public slots:
    void setStatus(THORQ_RELATIONSHIP_STATUS status);
    void setAuthority(THORQ_RELATIONSHIP_AUTHORITY authority);
private:
    int m_privId;
    Account* m_source;
    Account* m_target;
    std::atomic<THORQ_RELATIONSHIP_STATUS>    m_status;
    std::atomic<THORQ_RELATIONSHIP_AUTHORITY> m_authority;
};
}

#endif // RELATIONSHIP_H
