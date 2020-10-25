#ifndef SESSION_H
#define SESSION_H

#include <atomic>
#include <cstdint>
#include <unordered_set>
#include <tbb/concurrent_unordered_map.h>

#include <stduuid/include/uuid.h>

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
    uuids::uuid m_id;
    struct Participant
    {
        uuids::uuid m_id;
        std::shared_ptr<ThorQ::Account> account = nullptr;
        std::atomic_uint8_t permissions = 0;
    };
    tbb::concurrent_unordered_map<std::string, Participant> m_participants;
};
}

#endif // SESSION_H
