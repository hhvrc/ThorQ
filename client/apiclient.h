#ifndef APIAPICLIENT_H
#define APIAPICLIENT_H

#include <networking/client.h>

namespace ThorQ {
class ApiClient : public ThorQ::Networking::Tcp::Client
{
public:
    ApiClient();
    ~ApiClient();
private:
    void onCreatedConnection(std::shared_ptr<ThorQ::Networking::Tcp::Connection> connection) override;
};
}

#endif // APIAPICLIENT_H
