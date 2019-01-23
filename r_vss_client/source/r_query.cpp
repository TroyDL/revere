#include "r_vss_client/r_query.h"
#include "r_utils/r_socket.h"
#include "r_utils/r_time_utils.h"
#include "r_http/r_client_request.h"
#include "r_http/r_client_response.h"
#include "r_http/r_http_exception.h"

using namespace r_vss_client;
using namespace r_utils;
using namespace r_http;
using namespace std::chrono;
using namespace std;

vector<uint8_t> r_vss_client::query(const std::string& dataSourceID,
                                    const std::string& type,
                                    bool previousPlayable,
                                    const std::chrono::system_clock::time_point& start,
                                    const std::chrono::system_clock::time_point& end,
                                    bool keyOnly)
{
    r_client_request request("127.0.0.1", 11002);
#if 0
    request.set_uri(r_string_utils::format("/query?data_source_id=%s&type=%s&previous_playable=%s&start_time=%s&end_time=%s&key_only=%s",
                                     dataSourceID.c_str(),
                                     type.c_str(),
                                     (previousPlayable)?"true":"false",
                                     r_time_utils::tp_to_iso_8601(start, false).c_str(),
                                     r_time_utils::tp_to_iso_8601(end, false).c_str(),
                                     (keyOnly)?"true":"false"));
#else
    auto uri = r_string_utils::format("/query?data_source_id=%s&type=%s&previous_playable=%s&start_time=%s&end_time=%s&key_only=%s",
                                     dataSourceID.c_str(),
                                     type.c_str(),
                                     (previousPlayable)?"true":"false",
                                     r_time_utils::tp_to_iso_8601(start, false).c_str(),
                                     r_time_utils::tp_to_iso_8601(end, false).c_str(),
                                     (keyOnly)?"true":"false");
printf("%s\n",uri.c_str());
    request.set_uri(uri);
#endif

    r_buffered_socket<r_socket> sok;
    sok.inner().set_io_timeout(30000);
    sok.connect("127.0.0.1", 11002);

    request.write_request(sok);

    r_client_response response;
    response.read_response(sok);

    if(response.is_failure())
        throw_r_http_exception(response.get_status(), "HTTP error.");

    return response.release_body();
}

vector<uint8_t> r_vss_client::query(const string& uri)
{
    r_client_request request("127.0.0.1", 11002);
    request.set_uri(uri);

    r_buffered_socket<r_socket> sok;
    sok.inner().set_io_timeout(30000);
    sok.connect("127.0.0.1", 11002);

    request.write_request(sok);

    r_client_response response;
    response.read_response(sok);

    if(response.is_failure())
        throw_r_http_exception(response.get_status(), "HTTP error.");

    return response.release_body();
}
