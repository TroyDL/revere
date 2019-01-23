
#include "r_vss_client/r_ds.h"
#include "r_utils/r_socket.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_http/r_client_request.h"
#include "r_http/r_client_response.h"
#include "r_http/r_http_exception.h"
#include <algorithm>

using namespace r_utils;
using namespace r_http;
using json = nlohmann::json;
using namespace std;

string r_vss_client::fetch_data_sources_json()
{
    r_client_request request("127.0.0.1", 11002);
    request.set_uri("/data_sources");

    r_buffered_socket<r_socket> sok;
    sok.inner().set_io_timeout(30000);
    sok.connect("127.0.0.1", 11002);

    request.write_request(sok);

    r_client_response response;
    response.read_response(sok);

    if(response.is_failure())
        throw_r_http_exception(response.get_status(), "HTTP error.");

    return response.get_body_as_string();
}

vector<r_vss_client::data_source> r_vss_client::parse_data_sources(const string& ds)
{
    vector<r_vss_client::data_source> dataSources;

    auto doc = json::parse(ds);

    for(auto dsj : doc["data"]["data_sources"])
    {
        r_vss_client::data_source ds;
        ds.auth_password = dsj["auth_password"].get<string>();
        ds.auth_username = dsj["auth_username"].get<string>();
        ds.id = dsj["id"].get<string>();
        ds.recording = (r_string_utils::to_lower(dsj["recording"].get<string>()) == "true")?true:false;
        ds.rtsp_url = dsj["rtsp_url"].get<string>();
        ds.transport_pref = dsj["transport_pref"].get<string>();
        ds.type = dsj["type"].get<string>();
        dataSources.push_back(ds);
    }

    return dataSources;
}