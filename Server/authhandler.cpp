#include "authhandler.h"

#include <fstream>
#include <set>
#include <ctime>
#include <string>
#include <mutex>

struct UserAuth
{
    const std::string* hwid;
    const std::string* regkey;
    time_t lastLogin;
};

struct Redirect
{
    const std::string string;
    const UserAuth* userAuth;

    inline int  operator <  (const Redirect& other) const { return string <  other.string; };
    inline int  operator >  (const Redirect& other) const { return string >  other.string; };
    inline int  operator <= (const Redirect& other) const { return string <= other.string; };
    inline int  operator >= (const Redirect& other) const { return string >= other.string; };
    inline bool operator == (const Redirect& other) const { return string == other.string; };
    inline bool operator != (const Redirect& other) const { return string != other.string; };
};

static std::mutex mutex{};
static std::set<UserAuth> set_users{};
static std::set<Redirect> set_hwid{};
static std::set<Redirect> set_regkey{};

void insert(const std::string& hwid, const std::string& regkey)
{
    std::scoped_lock<std::mutex> lock(mutex);

    // create pairs
    auto it_hwid = set_hwid.insert({ hwid, nullptr });
    auto it_regkey = set_regkey.insert({ regkey, nullptr });

    if (it_hwid.second)
    {
        // HWID was available
    }

    if (it_regkey.second)
    {
        // Regkey was available
    }

    auto it_user = set_users.insert({ &it_hwid.first->string, &it_regkey.first->string, time(nullptr) });

    if (it_user.second)
    {
        // User was inserted
    }
}

void Init()
{
}

bool ThorQ::AuthHandler::CheckSystemID(const std::string& hwid)
{
    std::scoped_lock<std::mutex> lock(mutex);

    auto it = set_hwid.insert({ hwid, nullptr });

    return !it.second && it.first->userAuth != nullptr;
}

bool ThorQ::AuthHandler::TryRegisterHwid(const std::string& hwid, const std::vector<uint8_t>& key)
{
    (void)hwid;
    (void)key;
    return true;
}
