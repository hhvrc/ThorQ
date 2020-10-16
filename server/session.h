#ifndef SESSION_H
#define SESSION_H

#include <unordered_set>

#include "typedefs_server.h"

namespace ThorQ {
class Session
{
public:
    Session();
private:
    Account* m_leader;
    std::unordered_set<Account*> m_members;
};
}

#endif // SESSION_H
