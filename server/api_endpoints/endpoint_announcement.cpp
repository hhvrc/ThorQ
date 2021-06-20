#include "endpoint_announcement.h"

#include "messagehandlingcontext.h"

#include <schemas_common.h>
#include <fmt/core.h>

void ThorQ::ApiEndpoints::AnnouncementEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto fbsAnnouncement = context.body<ThorQ::Serialization::Announcement::Message>();

    fmt::print("[MSG] Announcement\n");
}
