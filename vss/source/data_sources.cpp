
#include "data_sources.h"
#include "r_db/r_sqlite_conn.h"
#include "r_utils/3rdparty/json/json.h"

using namespace r_utils;
using namespace r_http;
using namespace r_db;
using json = nlohmann::json;
using namespace std;

bool operator==(const data_source& a, const data_source& b)
{
    if((a.id != b.id) || (a.type != b.type) || (a.rtsp_url != b.rtsp_url) || (a.recording != b.recording) ||
       (a.transport_pref != b.transport_pref) || (a.auth_username != b.auth_username) || (a.auth_password != b.auth_password))
        return false;
    return true;
}

data_sources::data_sources(const string& dataSourcesPath) :
    _dataSourcesPath(dataSourcesPath)
{
    _upgrade_db(_dataSourcesPath);
}

r_server_response data_sources::handle_get(const r_web_server<r_socket>& ws,
                                           r_buffered_socket<r_socket>& conn,
                                           const r_server_request& request)
{
    r_sqlite_conn dbconn(_dataSourcesPath);

    json j;

    j["data"]["data_sources"] = json::array();

    auto results = dbconn.exec("SELECT * FROM data_sources;");

    for(auto r : results)
    {
        j["data"]["data_sources"] += {{"id", r["id"]},
                                      {"camera_id", r["camera_id"]},
                                      {"type", r["type"]},
                                      {"rtsp_url", r["rtsp_url"]},
                                      {"recording", r["recording"]},
                                      {"transport_pref", r["transport_pref"]},
                                      {"auth_username", r["auth_username"]},
                                      {"auth_password", r["auth_password"]}};
    }


    r_server_response response;

    response.set_body(j.dump());

    return response;
}

// curl -H 'Content-Type: application/json' -X PUT "http://127.0.0.1:11002/data_sources" -d "{ \"id\": \"1234\", \"type\": \"video\", \"rtsp_url\": \"rtsp://127.0.0.1:554/stream1\", \"recording_video\": \"true\", \"recording_audio\": \"false\", \"recording_md\": \"true\", \"transport_pref\": \"tcp\", \"auth_username\": \"dicroce\", \"auth_password\": \"emperor1\" }"

r_server_response data_sources::handle_put(const r_web_server<r_socket>& ws,
                                           r_buffered_socket<r_socket>& conn,
                                           const r_server_request& request)
{
    auto j = json::parse(request.get_body_as_string());

    r_sqlite_conn dbconn(_dataSourcesPath, true);

    dbconn.exec(r_string_utils::format("REPLACE INTO data_sources(id, camera_id, type, rtsp_url, recording, transport_pref, auth_username, auth_password) "
                                       "VALUES(%s, %s, '%s', '%s', '%s', '%s', '%s', '%s');",
                                       j["id"].get<string>().c_str(),
                                       j["camera_id"].get<string>().c_str(),
                                       j["type"].get<string>().c_str(),
                                       j["rtsp_url"].get<string>().c_str(),
                                       j["recording"].get<string>().c_str(),
                                       j["transport_pref"].get<string>().c_str(),
                                       j["auth_username"].get<string>().c_str(),
                                       j["auth_password"].get<string>().c_str()));

    return r_server_response();
}

r_server_response data_sources::handle_del(const r_web_server<r_socket>& ws,
                                           r_buffered_socket<r_socket>& conn,
                                           const r_server_request& request)
{
    auto j = json::parse(request.get_body_as_string());

    r_sqlite_conn dbconn(_dataSourcesPath, true);

    dbconn.exec(r_string_utils::format("DELETE FROM data_sources WHERE id=%s;",
                                 j["id"].get<string>().c_str()));

    return r_server_response();
}

vector<data_source> data_sources::recording_data_sources()
{
    r_sqlite_conn conn(_dataSourcesPath);

    vector<data_source> sources;
    auto results = conn.exec("SELECT * FROM data_sources;");

    for(auto row : results)
    {
        if(r_string_utils::to_lower(row["recording"]) == "true")
        {
            data_source ds;
            ds.id = row["id"];
            ds.camera_id = row["camera_id"];
            ds.type = row["type"];
            ds.rtsp_url = row["rtsp_url"];
            ds.recording = true;
            ds.transport_pref = row["transport_pref"];
            ds.auth_username = row["auth_username"];
            ds.auth_password = row["auth_password"];
            sources.push_back(ds);
        }
    }

    return sources;
}

void data_sources::_upgrade_db(const string& dataSourcesPath)
{
    r_sqlite_conn conn(dataSourcesPath, true);

    auto results = conn.exec("PRAGMA user_version;");

    uint16_t version = 0;

    if(!results.empty())
        version = r_string_utils::s_to_uint16(results.front()["user_version"]);

    switch( version )
    {

    case 0:
    {
        R_LOG_NOTICE("upgrade data_sources to version: 1");
        conn.exec("CREATE TABLE IF NOT EXISTS data_sources(id VARCHAR PRIMARY KEY, "
                                                    "camera_id VARCHAR, "
                                                    "type VARCHAR, "
                                                    "rtsp_url VARCHAR, "
                                                    "recording VARCHAR, "
                                                    "transport_pref VARCHAR, "
                                                    "auth_username VARCHAR, "
                                                    "auth_password VARCHAR);");
        conn.exec("PRAGMA user_version=1;");
    }

    default:
        break;
    }
}
