#include "endpoint_systemid.h"

#include "apiserver_connection.h"
#include "messagehandlingcontext.h"
#include "database.h"

#include <systemid.h>
#include <schemas_common.h>

#include <lsql/connection.h>
#include <lsql/column.h>

#include <fmt/core.h>

using namespace std::literals;

void ThorQ::ApiEndpoints::SystemidEndpoint::handleMessage(ThorQ::HandlerContext& context)
{
    auto msgBody = context.body<ThorQ::Serialization::SystemId::Message>();
    auto connection = context.apiConnection();

    if (msgBody->cmd() != ThorQ::Serialization::SystemId::Command_Submit)
    {
        // TODO: THORQ_DISCONNECT_REASON::INVALID_OPERATION
        connection->disconnect();
        return;
    }

    std::span<std::uint8_t> systemid(const_cast<std::uint8_t*>(msgBody->data()->data()), msgBody->data()->size());

    if (!ThorQ::SystemID::systemid_validate(systemid))
    {
        fmt::print("[MSG] Got forged SystemID!\n");
        // TODO: THORQ_DISCONNECT_REASON::INVALID_HWID
        connection->disconnect();
        return;
    }

    std::string systemID = ThorQ::SystemID::systemid_to_string(systemid);

    fmt::print("SystemID: {}\n", systemID);

    auto dbConnection = openDatabaseConneciton(SQLite::Connection::READWRITE);
    if (dbConnection == nullptr)
    {
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        connection->disconnect();
        return;
    }

    auto dbInsertSystemId = dbConnection->makeQuery("INSERT OR IGNORE INTO systems(hardware_id) VALUES (?);"sv);
    dbInsertSystemId.bindText(1, systemID);

    if (!dbInsertSystemId.step())
    {
        fmt::print(stderr, "[SQLITE] Failed to insert system: {}\n", dbConnection->lastError());
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        connection->disconnect();
        return;
    }

    auto dbCheckSystemIdBanned = dbConnection->makeQuery(
                "SELECT COUNT(*) FROM accounts WHERE account_id IN ("
                "SELECT account_id FROM account_systems WHERE system_id IN ("
                "SELECT system_id FROM systems WHERE hardware_id = ?"
                ")) AND banned_at IS NOT NULL LIMIT 1"sv);
    dbCheckSystemIdBanned.bindText(1, systemID);

    if (!dbCheckSystemIdBanned.step())
    {
        fmt::print(stderr, "[SQLITE] Failed to select banned_at: {}\n", dbConnection->lastError());
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        connection->disconnect();
        return;
    }

    if (dbCheckSystemIdBanned.columnCount() == 0)
    {
        fmt::print(stderr, "[SQLITE] SQLite query returned invalid amount of rows ({})\n", dbCheckSystemIdBanned.columnCount());
        // TODO: THORQ_DISCONNECT_REASON::SERVER_ERROR
        connection->disconnect();
        return;
    }

    bool isBanned = (dbCheckSystemIdBanned.column(0).getInt32() > 0);

    if (isBanned)
    {
        fmt::print("Connection is banned!\n");
        // TODO: THORQ_DISCONNECT_REASON::BANNED
        connection->disconnect();
        return;
    }

    fmt::print("Connection is ok!\n");
}
