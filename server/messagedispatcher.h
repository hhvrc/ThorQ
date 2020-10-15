#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include "typedefs_global.h"

namespace ThorQ {
class MessageDispatcher
{
public:
    MessageDispatcher();

    void DispatchMessage(ENetEvent event);
private:
};
}

#endif // MESSAGEDISPATCHER_H
