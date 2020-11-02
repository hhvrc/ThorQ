#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include <thread>
#include <vector>

#include "concurrentqueue.h"

#include "constants.h"
#include "typedefs_global.h"
#include "typedefs_server.h"

namespace ThorQ {
class MessageDispatcher
{
    friend ThorQ::Server;
    MessageDispatcher(ThorQ::Server* server);
public:
    void DispatchEvent(const ENetEvent& event);
private:
    Server* m_server;
    std::thread* m_thread;
    std::vector<std::uint8_t> m_buffer;
    moodycamel::ConsumerToken m_tokenGet;
    moodycamel::ProducerToken m_tokenQueue;
    moodycamel::ProducerToken m_tokenBroadcast;
    moodycamel::ProducerToken m_tokenDisconnect;
};
}

#endif // MESSAGEDISPATCHER_H
