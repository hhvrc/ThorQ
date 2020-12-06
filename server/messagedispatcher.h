#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include <span>
#include <thread>
#include <vector>
#include <cstdint>

#include <concurrentqueue.h>

#include <enums.h>
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

    void handleMessageVersion(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageHeartbeat(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageUser(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageFile(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageCrypto(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageSystemID(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageAccount(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageFriendRequest(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageGroup(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageModeration(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);
    void handleMessageCollar(ThorQ::Instance* instance, const void* body, flatbuffers::Verifier fbsVerifier);

    void sendPacket(ThorQ::Instance* instance, std::span<uint8_t> data, bool encrypt, uint32_t flags, THORQ_CHANNEL channel);

    Server* m_server;
    std::thread m_thread;
    std::atomic_bool m_closing;
    std::vector<std::uint8_t> m_buffer;
    moodycamel::ConsumerToken m_tokenGet;
    moodycamel::ProducerToken m_tokenQueue;
};
}

#endif // MESSAGEDISPATCHER_H
