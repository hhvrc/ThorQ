#include "endpoint_file.h"

#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::FileEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto fbsFile = context.body<ThorQ::Serialization::File::Message>();

    fmt::print("[MSG] File\n");
}
