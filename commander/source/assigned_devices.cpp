
#include "assigned_devices.h"
#include "r_db/r_sqlite_conn.h"
#include "r_utils/3rdparty/json/json.h"

using namespace r_utils;
using namespace r_http;
using namespace r_db;
using json = nlohmann::json;
using namespace std;

bool operator==(const assigned_device& a, const assigned_device& b)
{
    if((a.id != b.id) || (a.type != b.type) || (a.rtsp_url != b.rtsp_url))
        return false;
    return true;
}

assigned_devices::assigned_devices(const string& devicesPath) :
    _devicesPath(devicesPath)
{
    _upgrade_db(_devicesPath);
}

r_server_response assigned_devices::handle_get(const r_web_server<r_socket>& ws,
                                           r_buffered_socket<r_socket>& conn,
                                           const r_server_request& request)
{
    r_sqlite_conn dbconn(_devicesPath);

    json j;

    j["data"]["assigned_devices"] = json::array();

    auto results = dbconn.exec("SELECT * FROM assigned_devices;");

    for(auto r : results)
    {
        j["data"]["assigned_devices"] += {{"id", r["id"]},
                                          {"type", r["type"]},
                                          {"rtsp_url", r["rtsp_url"]}};
    }

    r_server_response response;

    response.set_body(j.dump());

    return response;
}

r_server_response assigned_devices::handle_put(const r_web_server<r_socket>& ws,
                                               r_buffered_socket<r_socket>& conn,
                                               const r_server_request& request)
{
    auto j = json::parse(request.get_body_as_string());

    r_sqlite_conn dbconn(_devicesPath, true);

    dbconn.exec(r_string_utils::format("REPLACE INTO assigned_devices(id, type, rtsp_url) "
                                       "VALUES('%s', '%s', '%s');",
                                       j["id"].get<string>().c_str(),
                                       j["type"].get<string>().c_str(),
                                       j["rtsp_url"].get<string>().c_str()));
    return r_server_response();
}

r_server_response assigned_devices::handle_del(const r_web_server<r_socket>& ws,
                                               r_buffered_socket<r_socket>& conn,
                                               const r_server_request& request)
{
    auto j = json::parse(request.get_body_as_string());

    r_sqlite_conn dbconn(_devicesPath, true);

    dbconn.exec(r_string_utils::format("DELETE FROM assigned_devices WHERE id='%s';",
                                       j["id"].get<string>().c_str()));

    return r_server_response();
}

map<string, assigned_device> assigned_devices::get()
{
    r_sqlite_conn conn(_devicesPath);

    map<string, assigned_device> sources;
    auto results = conn.exec("SELECT * FROM assigned_devices;");

    for(auto row : results)
    {
        assigned_device ds;
        ds.id = row["id"];
        ds.type = row["type"];
        ds.rtsp_url = row["rtsp_url"];
        sources[ds.id] = ds;
    }

    return sources;
}

void assigned_devices::_upgrade_db(const string& devicesPath)
{
    r_sqlite_conn conn(devicesPath, true);

    auto results = conn.exec("PRAGMA user_version;");

    uint16_t version = 0;

    if(!results.empty())
        version = r_string_utils::s_to_uint16(results.front()["user_version"]);

    switch( version )
    {

    case 0:
    {
        R_LOG_NOTICE("upgrade assigned_devices to version: 1");
        conn.exec("CREATE TABLE IF NOT EXISTS assigned_devices(id VARCHAR PRIMARY KEY, "
                                                              "type VARCHAR, "
                                                              "rtsp_url VARCHAR);");
        conn.exec("PRAGMA user_version=1;");
    }

    default:
        break;
    }
}
