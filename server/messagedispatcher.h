#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include <thread>
#include <vector>

#include "constants.h"
#include "typedefs_global.h"
#include "typedefs_server.h"

namespace ThorQ {
class MessageDispatcher
{
public:
    MessageDispatcher(Server* serverInstance);

    void DispatchEvent(const ENetEvent& event);
private:
    Server* m_server;
    std::thread* m_thread;
    std::vector<std::uint8_t> m_buffer;
};
}

#endif // MESSAGEDISPATCHER_H
