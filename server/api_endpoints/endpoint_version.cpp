#include "endpoint_version.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <enums.h>
#include <constants.h>
#include <version.h>
#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::VersionEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Version>();
    auto connection = context.apiConnection();

    fmt::print("[MSG] version\n");

    ThorQ::Version version;
    version.setMajor(msgBody->major());
    version.setMinor(msgBody->minor());
    version.setPatch(msgBody->patch());

    switch ((THORQ_APP)msgBody->app()) {
    case THORQ_APP::SERVER:
        if (version == ThorQ::ServerVersion) {
            fmt::print("[VERSION] Server version matched\n");
        }
        else {
            fmt::print("[VERSION] Server version mismatched\n");
            // TODO: THORQ_DISCONNECT_REASON::VERSION_MISMATCH
            connection->disconnect();
        }
        break;
    case THORQ_APP::CLIENT:
        if (version == ThorQ::ClientVersion) {
            fmt::print("[VERSION] Client version matched\n");
        }
        else {
            fmt::print("[VERSION] Client version mismatched\n");
            // TODO: THORQ_DISCONNECT_REASON::VERSION_MISMATCH
            connection->disconnect();
        }
        break;
    case THORQ_APP::LINK:
        if (version == ThorQ::LinkVersion) {
            fmt::print("[VERSION] Link version matched\n");
        }
        else {
            fmt::print("[VERSION] Link version mismatched\n");
            // TODO: THORQ_DISCONNECT_REASON::VERSION_MISMATCH
            connection->disconnect();
        }
        break;
    default:
        fmt::print("[VERSION] invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        connection->disconnect();
        break;
    }
}
