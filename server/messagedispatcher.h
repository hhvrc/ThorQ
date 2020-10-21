#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include <thread>
#include <array>

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
    std::array<std::uint8_t, THORQ_PAYLOAD_LEN_MAX> m_buffer;
};
}

#endif // MESSAGEDISPATCHER_H
