#ifndef SESSION_H
#define SESSION_H

#include <atomic>
#include <cstdint>
#include <shared_mutex>
#include <unordered_map>

#include "uuid.h"
#include "typedefs_server.h"

namespace ThorQ {
class Session
{
public:
    enum class Type
    {
        Direct,
        FreeForAll,
    } ;

    Session();
private:
    ThorQ::Uuid m_id;
    struct Participant
    {
        ThorQ::Uuid m_id;
        std::shared_ptr<ThorQ::Account> account = nullptr;
        std::atomic_uint8_t permissions = 0;
    };
    std::shared_mutex l_participants;
    std::unordered_map<std::string, Participant> m_participants;
};
}

#endif // SESSION_H
