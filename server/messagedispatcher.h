#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include <thread>
#include <vector>
#include <cstdint>

#include <concurrentqueue.h>

#include <constants.h>
#include <typedefs_global.h>

#include "typedefs_server.h"

namespace ThorQ {
class MessageDispatcher
{
public:
    MessageDispatcher(ThorQ::Server* server);
    ~MessageDispatcher();

private:
    void run();

    void handleEventConnection(const ENetEvent& event);
    void handleEventMessage(const ENetEvent &event);
    void handleEventDisconnect(const ENetEvent& event);
    void handleEventTimeout(const ENetEvent& event);

    void handleMessageVersion(ThorQ::Instance* instance, const ThorQ::Serialization::Version* fbsVersion, flatbuffers::Verifier fbsVerifier);
    void handleMessageHeartbeat(ThorQ::Instance* instance, const ThorQ::Serialization::Heartbeat* fbsHeartbeat, flatbuffers::Verifier fbsVerifier);
    void handleMessageCrypto(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageSystemID(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageAccount(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageRelation(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageSession(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageModeration(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageAnnouncement(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageToy(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageCollar(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);
    void handleMessageAck(ThorQ::Instance* instance, const std::vector<std::uint8_t>& message);


    Server* m_server;
    std::thread m_thread;
    std::atomic_bool m_closing;
    std::vector<std::uint8_t> m_buffer;
    moodycamel::ConsumerToken m_tokenGet;
    moodycamel::ProducerToken m_tokenQueue;
};
}

#endif // MESSAGEDISPATCHER_H
