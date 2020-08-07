#include "authhandler.h"

#include <fstream>
#include <set>
#include <ctime>
#include <string>
#include <json.hpp>

struct UserAuth
{
    std::string hwid;
    std::string authToken;
    std::int64_t lastLogin;
};

static std::set<UserAuth> authUsers;
static std::map<std::string, UserAuth> hwids;

void eeeee()
{
    try
    {
        nlohmann::json j;

        std::fstream file("auth.json", std::fstream::binary | std::fstream::in | std::fstream::ate);

        std::string content;
        content.resize(file.tellg());
        file.seekg(0, std::fstream::beg);

        file.read(&content[0], content.size());

        file.close();
    }
	catch (const std::exception& ex)
    {
        printf("Oppsie!");
    }
}

bool ThorQ::AuthHandler::CheckSystemID(const std::string& hwid)
{
    (void)hwid;
    return false;
}

bool ThorQ::AuthHandler::TryRegisterHwid(const std::string& hwid, const std::vector<uint8_t>& key)
{
    (void)hwid;
    (void)key;
    return true;
}
