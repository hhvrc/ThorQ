#include "authhandler.h"

#include <fstream>
#include <set>
#include <string>
#include <json.hpp>

AuthHandler::AuthHandler()
{
    try
    {
        nlohmann::json j;

        std::fstream file("auth.json", std::fstream::binary | std::fstream::in | std::fstream::ate);

        std::string content;
        content.resize(file.tellg());
        file.seekg(0, std::fstream::beg);

        file.read(content.data(), content.size());

        file.close();
    }
    catch (std::exception ex)
    {
        printf("Oppsie!");
    }
}
