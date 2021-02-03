#include <uuid.h>

#include <fmt/core.h>

#include <vector>
#include <cstring>
#include <cstdint>

int main()
{
    ThorQ::Uuid uuid = ThorQ::Uuid::NewUuid();

    ThorQ::Uuid fromBytes(uuid.toBytes());
    if (uuid != fromBytes) {
        fmt::print("FromBytes failed!\n");
        return EXIT_FAILURE;
    }

    ThorQ::Uuid fromString;
    ThorQ::Uuid::TryParse(uuid.toString(), fromString);
    if (uuid != fromString) {
        fmt::print("FromString failed!\n");
        return EXIT_FAILURE;
    }

    fmt::print("Success\n");
    return EXIT_SUCCESS;
}
