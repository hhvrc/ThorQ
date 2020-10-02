#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include <QObject>

#include "typedefs_global.h"

namespace ThorQ {
class MessageDispatcher : public QObject
{
    Q_OBJECT
public:
    MessageDispatcher(QObject* parent = nullptr);
public slots:
    void DispatchMessage(ENetEvent event);
private:
};
}

#endif // MESSAGEDISPATCHER_H
