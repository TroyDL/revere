
#include "test_r_http_r_web_server.h"
#include "r_http/r_web_server.h"
#include "r_http/r_client_request.h"
#include "r_http/r_client_response.h"

using namespace std;
using namespace r_utils;
using namespace r_http;

REGISTER_TEST_FIXTURE(test_r_http_r_web_server);

void test_r_http_r_web_server::setup()
{
}

void test_r_http_r_web_server::teardown()
{
}


r_client_response _send_request(int port, r_client_request& request)
{
    r_socket socket;
    socket.connect("127.0.0.1", port);
    request.write_request(socket);

    r_client_response response;
    response.read_response(socket);

    return response;
}

void test_r_http_r_web_server::test_basic()
{
    int port = RTF_NEXT_PORT();

    r_web_server<r_socket> ws( port );

    ws.add_route(METHOD_GET, "/hi", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        r_server_response response;
        response.set_body("hello");
        return response;
    });

    ws.start();

    usleep(100000);

    r_client_request request("127.0.0.1", port);
    request.set_uri("/hi");

    auto response = _send_request(port, request);

    RTF_ASSERT(response.get_body_as_string() == "hello" );
}

void test_r_http_r_web_server::test_partial_route()
{
    int port = RTF_NEXT_PORT();

    r_web_server<r_socket> ws( port );

    ws.add_route(METHOD_GET, "/hi", [](const r_web_server<r_socket>& ws, r_utils::r_buffered_socket<r_utils::r_socket>& conn, const r_server_request& request)->r_server_response {
        auto resource = request.get_uri().get_resource();
        r_server_response response;
        response.set_content_type("text/plain");
        response.set_body(resource);
        return response;
    });

    ws.start();

    usleep(100000);

    r_client_request request("127.0.0.1", port);
    request.set_uri("/hi/hello");
    auto response = _send_request(port, request);

    RTF_ASSERT(response.get_body_as_string() == "hello" );

}