#ifndef SESSION_H
#define SESSION_H

#include <atomic>
#include <cstdint>
#include <shared_mutex>
#include <unordered_map>

#include <uuid.h>

#include "typedefs_server.h"

namespace ThorQ {
class Group
{
public:
    enum class Type
    {
        Master,
        FreeForAll
    } ;

    Group();
private:
    ThorQ::Uuid m_id;
    std::string m_name;
    ThorQ::Uuid m_image;

    struct Participant
    {
        std::shared_ptr<ThorQ::Account> account = nullptr;
        std::atomic_uint8_t max_strength = 100;
    };

    std::shared_mutex l_participants;
    std::unordered_map<std::string, Participant> m_participants;
};
}

#endif // SESSION_H
