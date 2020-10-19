#ifndef SESSION_H
#define SESSION_H

#include <unordered_set>

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
    struct Participant
    {
        Account* account;
        std::uint8_t permissions;
    };
    std::unordered_set<Participant> m_participants;
};
}

#endif // SESSION_H
