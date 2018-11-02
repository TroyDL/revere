
#include "r_rtsp/r_fake_camera.h"
#include "r_av/r_locky.h"
#include "r_utils/r_logger.h"
#include "r_http/r_web_server.h"
#include <signal.h>
#include <unistd.h>

using namespace r_utils;
using namespace r_http;
using namespace std;

const int SERVER_PORT = 11004;
const int RTSP_PORT = 11005;

bool running = true;

void handle_sigterm(int sig)
{
    if(sig == SIGTERM)
        running = false;
}

int main(int argc, char* argv[])
{
    r_logger::install_terminate();

    r_av::r_locky::register_ffmpeg();

    signal(SIGTERM, handle_sigterm);

    string firstArgument = (argc > 1) ? argv[1] : string();

    if( firstArgument != "--interactive" )
        daemon( 1, 0 );

    r_rtsp::r_fake_camera fc(".", 11005);
    fc.start();

    r_web_server<r_socket> ws(SERVER_PORT);

    ws.add_route(METHOD_GET, "/status", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        response.set_body("{ \"status\": \"healthy\" }");
        return response;
    });

    ws.start();

    while(running)
    {
        usleep(500000);
    }

    fc.stop();

    r_av::r_locky::unregister_ffmpeg();

    return 0;
}
