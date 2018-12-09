
#include "r_http/r_web_server.h"
#include "r_http/r_http_exception.h"
#include "r_disco/r_discovery.h"
#include <signal.h>
#include <unistd.h>

using namespace r_utils;
using namespace r_http;
using namespace r_disco;
using namespace std;

static const int SERVER_PORT = 11004;

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

    string firstArgument = (argc > 1) ? argv[1] : string();

    if( firstArgument != "--interactive" )
        daemon( 1, 0 );

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

    return 0;
}
