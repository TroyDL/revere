
#include "test_r_rtsp_r_client_connection.h"
#include "r_rtsp/r_client_connection.h"
#include "r_rtsp/r_client_request.h"
#include "r_rtsp/r_server_request.h"
#include <thread>

using namespace std;
using namespace r_utils;
using namespace r_rtsp;

REGISTER_TEST_FIXTURE(test_r_rtsp_r_client_connection);

void test_r_rtsp_r_client_connection::setup()
{
}

void test_r_rtsp_r_client_connection::teardown()
{
}

void test_r_rtsp_r_client_connection::test_connect()
{
    int port = RTF_NEXT_PORT();

    // For some reason, my first impulse is to always make the thread be the server... but the nice thing about making the
    // thread be the client is that you don't need a sleep!

    // Also, you should probably always call fut.get() to propigate any exceptions thrown in the async task.

    thread t([&](){
        usleep(1000000);
        r_client_connection cc( "127.0.0.1", port );
        cc.connect();
        shared_ptr<r_client_request> req = make_shared<r_client_request>( M_OPTIONS );
        req->set_uri( "/foo/bar" );
        cc.write_request( req );
    });
    t.detach();

    r_socket server_sok;
    server_sok.bind( port, ip4_addr_any );
    server_sok.listen();
    auto connected = server_sok.accept();
    r_server_request sreq;
    sreq.read_request( connected );

    RTF_ASSERT( sreq.get_uri() == "/foo/bar" );
}
