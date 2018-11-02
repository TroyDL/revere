
#include "test_r_rtsp_r_client_request.h"
#include "r_rtsp/r_client_connection.h"
#include "r_rtsp/r_client_request.h"
#include "r_rtsp/r_server_request.h"
#include <thread>
#include <future>

using namespace std;
using namespace r_utils;
using namespace r_rtsp;

REGISTER_TEST_FIXTURE(test_r_rtsp_r_client_request);

void test_r_rtsp_r_client_request::setup()
{
}

void test_r_rtsp_r_client_request::teardown()
{
}

void test_r_rtsp_r_client_request::test_constructor()
{
    {
        r_client_request req;
    }

    {
        r_client_request reqA;
        r_client_request reqB = reqA;
    }

    {
        r_server_request req;
    }

    {
        r_server_request reqA;
        r_server_request reqB = reqA;
    }
}

void test_r_rtsp_r_client_request::test_assignment_op()
{
    r_client_request reqA;
    reqA.set_uri("/foo/bar");
    reqA.set_header( "word", "up" );
    r_client_request reqB;
    reqB = reqA;
    RTF_ASSERT( reqB.get_uri() == "/foo/bar" );
    string value;
    RTF_ASSERT( reqB.get_header( "word", value ) );
    RTF_ASSERT( value == "up" );
}

void test_r_rtsp_r_client_request::test_set_method()
{
    r_client_request reqA;
    reqA.set_method( M_OPTIONS );
    RTF_ASSERT( reqA.get_method() == M_OPTIONS );
}

void test_r_rtsp_r_client_request::test_set_header()
{
    r_client_request reqA;
    reqA.set_header( "name", "value" );
    string value;
    RTF_ASSERT( reqA.get_header( "name", value ) );
    RTF_ASSERT( value == "value" );
}

void test_r_rtsp_r_client_request::test_set_user_agent()
{
    r_client_request req;
    req.set_user_agent( "your_mom" );
    RTF_ASSERT( req.get_user_agent() == "your_mom" );
}

void test_r_rtsp_r_client_request::test_set_server_port()
{
    r_client_request req;
    req.set_server_port( 42 );
    RTF_ASSERT( req.get_server_port() == 42 );
}

void test_r_rtsp_r_client_request::test_write_request()
{
    int port = RTF_NEXT_PORT();

    // For some reason, my first impulse is to always make the thread be the server... but the nice thing about making the
    // thread be the client is that you don't need a sleep!

    // Also, you should probably always call fut.get() to propigate any exceptions thrown in the async task.

    auto fu = std::async(std::launch::async, [&](){
        usleep(1000000);
        r_client_connection cc( "127.0.0.1", port );
        cc.connect();
        shared_ptr<r_client_request> req = make_shared<r_client_request>( M_DESCRIBE );
        req->set_uri( "/foo/bar" );
        req->set_header( "your", "mom" );
        req->set_user_agent( "lord_stark" );
        cc.write_request( req );
    });

    r_socket server_sok;
    server_sok.bind( port, ip4_addr_any );
    server_sok.listen();
    auto connected = server_sok.accept();
    r_server_request sreq;
    sreq.read_request( connected );

    RTF_ASSERT( sreq.get_method() == M_DESCRIBE );
    RTF_ASSERT( sreq.get_uri() == "/foo/bar" );
    string value;
    RTF_ASSERT( sreq.get_header( "your", value ) );
    RTF_ASSERT( value == "mom" );
    RTF_ASSERT( sreq.get_header( "User-Agent", value ) );
    RTF_ASSERT( value == "lord_stark" );

    fu.get(); // about it....
}