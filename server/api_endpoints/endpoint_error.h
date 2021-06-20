#ifndef ENDPOINT_ERROR_H
#define ENDPOINT_ERROR_H

#include <typedefs_global.h>
#include <typedefs_server.h>

namespace ThorQ::ApiEndpoints {
class ErrorEndpoint
{
public:
    ErrorEndpoint();
    ~ErrorEndpoint();

    void handleMessage(HandlerContext& context);
private:
};
}

#endif // ENDPOINT_ERROR_H
