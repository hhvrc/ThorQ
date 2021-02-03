#include <systemid.h>

#include <fmt/core.h>

#include <vector>
#include <cstring>
#include <cstdint>

int main()
{
    auto systemID = ThorQ::SystemID::systemid_generate();

    if (!ThorQ::SystemID::systemid_validate(systemID)) {
        fmt::print("Failed to verify SystemID!\n");
        return EXIT_FAILURE;
    }

    std::string str = ThorQ::SystemID::systemid_to_string(systemID);
    fmt::print("SystemID: {}\n", str.c_str());

    return EXIT_SUCCESS;
}
