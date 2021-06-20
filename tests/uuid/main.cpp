#include <uuid.h>

#include <fmt/core.h>

#include <vector>
#include <cstring>
#include <cstdint>

int main()
{
    for (int i = 0; i < 1000; i++) {
        ThorQ::Uuid uuid = ThorQ::Uuid::NewUuid();
        std::string uuidStr = uuid.toString();

        fmt::print("{}\n", uuidStr);

        ThorQ::Uuid fromBytes(uuid.toBytes());
        if (uuid != fromBytes) {
            fmt::print("FromBytes failed!\n");
            return EXIT_FAILURE;
        }

        ThorQ::Uuid fromString;
        ThorQ::Uuid::TryParse(uuid.toString().c_str(), fromString);
        if (uuid != fromString) {
            fmt::print("FromString failed!\n");
            return EXIT_FAILURE;
        }
    }

    fmt::print("OK\n");
    return EXIT_SUCCESS;
}
