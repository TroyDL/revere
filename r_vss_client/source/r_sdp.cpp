
#include "r_vss_client/r_sdp.h"
#include "r_utils/r_socket.h"
#include "r_utils/r_time_utils.h"
#include "r_http/r_client_request.h"
#include "r_http/r_client_response.h"
#include "r_http/r_http_exception.h"
#include <algorithm>

using namespace r_utils;
using namespace r_http;
using namespace std;
using namespace std::chrono;

string r_vss_client::fetch_sdp_before(const string& dataSourceID,
                                      const string& type,
                                      const system_clock::time_point& time)
{
    r_client_request request("127.0.0.1", 11002);
    request.set_uri(r_string::format("/sdp_before?data_source_id=%s&time=%s&type=%s",
                                     dataSourceID.c_str(),
                                     r_time::tp_to_iso_8601(time, false).c_str(),
                                     type.c_str()));

    r_socket sok;
    sok.connect("127.0.0.1", 11002);

    request.write_request(sok);

    r_client_response response;
    response.read_response(sok);

    if(response.is_failure())
        throw_r_http_exception(response.get_status(), "HTTP error.");

    return response.get_body_as_string();
}
