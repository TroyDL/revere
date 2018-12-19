
#include "utils.h"
#include "r_utils/r_socket.h"
#include "r_http/r_client_request.h"
#include "r_http/r_client_response.h"
#include "r_utils/3rdparty/json/json.h"

using namespace std;
using namespace r_utils;
using namespace r_http;
using json = nlohmann::json;

vector<string> get_recording_camera_ids()
{
    vector<string> recordingIDs;

    string responseBody;

    try
    {
        r_socket sok;
        sok.connect("127.0.0.1", 11002);

        r_client_request req("127.0.0.1", 11002);
        req.set_uri("/data_sources");

        req.write_request(sok);

        r_client_response res;
        res.read_response(sok);

        responseBody = res.get_body_as_string();

        auto j = json::parse(responseBody);

        for(auto ds : j["data"]["data_sources"])
        {
            if(ds["recording"].get<string>() == "true")
                recordingIDs.push_back(ds["id"].get<string>());
        }
    }
    catch(exception& ex)
    {
        R_LOG_NOTICE("%s", ex.what());
    }

    return recordingIDs;
}
