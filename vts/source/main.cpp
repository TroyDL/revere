
#include "r_utils/r_logger.h"
#include "r_utils/r_file.h"
#include "r_utils/r_time_utils.h"
#include "r_http/r_web_server.h"
#include "r_vss_client/r_frame.h"
#include "r_vss_client/r_query.h"
#include "r_av/r_video_decoder.h"
#include "r_av/r_video_encoder.h"
#include "r_av/r_locky.h"
#include <string>
#include <chrono>
#include <signal.h>
#include <unistd.h>

#include "video.h"
#include "jpg.h"
#include "stream_segment.h"

using namespace r_utils;
using namespace r_http;
using namespace r_av;
using namespace std;
using namespace std::chrono;

static const int SERVER_PORT = 11003;

bool running = true;

void handle_sigterm(int sig)
{
    if(sig == SIGTERM)
        running = false;
}

int main(int argc, char* argv[])
{
    r_logger::install_terminate();

    signal(SIGTERM, handle_sigterm);

    r_locky::register_ffmpeg();

    string firstArgument = (argc > 1) ? argv[1] : string();

    if( firstArgument != "--interactive" )
        daemon( 1, 0 );

    r_web_server<r_socket> ws(SERVER_PORT);

    ws.add_route(METHOD_GET, "/status", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        response.set_body("{ \"status\": \"healthy\" }");
        return response;
    });

    ws.add_route(METHOD_GET, "/jpg_before", make_jpg);

    ws.add_route(METHOD_GET, "/video", make_video);

    ws.add_route(METHOD_GET, "/stream_segment", make_stream_segment);

    ws.add_route(METHOD_GET, "/test.html", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        response.set_body(r_fs::read_file("test.html"));
        response.set_content_type("text/html");
        return response;
    });

    ws.start();

    while(running)
    {
        usleep(500000);
    }

    r_locky::unregister_ffmpeg();

    return 0;
}