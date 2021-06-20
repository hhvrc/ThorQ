#include "endpoint_device.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::DeviceEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::Device::Message>();
    auto subBody = msgBody->body();
    auto connection = context.apiConnection();

    fmt::print("[MSG] Device\n");

    if (subBody == nullptr) {
        connection->disconnect();
        return;
    }

    context.setBody(subBody);

    switch (msgBody->body_type()) {
    case ThorQ::Serialization::Device::Body_device:
        break;
    default:
        fmt::print("[MSG][DEVICE] Invalid\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_REQUEST
        connection->disconnect();
        break;
    }
}
