
#include "test_r_rtsp_r_server_response.h"
#include "r_rtsp/r_server_response.h"
#include "r_rtsp/r_client_response.h"

#include <thread>
#include <future>

using namespace std;
using namespace r_utils;
using namespace r_rtsp;

REGISTER_TEST_FIXTURE(test_r_rtsp_r_server_response);

void test_r_rtsp_r_server_response::setup()
{
}

void test_r_rtsp_r_server_response::teardown()
{
}

void test_r_rtsp_r_server_response::test_constructor()
{
    {
        r_server_response res;
    }

    {
        r_server_response resA;
        r_server_response resB = resA;
    }
}

void test_r_rtsp_r_server_response::test_assignment_op()
{
    r_server_response resA;
    resA.set_status( STATUS_CONTINUE );
    resA.set_header( "word", "up" );
    resA.set_body( "this is a body" );
    r_server_response resB;
    resB = resA;
    RTF_ASSERT( resB.get_status() == STATUS_CONTINUE );
    string value;
    RTF_ASSERT( resB.get_header( "word", value ) );
    RTF_ASSERT( value == "up" );
    string responseBody = resB.get_body_as_string();
    RTF_ASSERT( responseBody == "this is a body" );
}

void test_r_rtsp_r_server_response::test_set_status()
{
    r_server_response resA;
    resA.set_status( STATUS_CONTINUE );
    RTF_ASSERT( resA.get_status() == STATUS_CONTINUE );
}

void test_r_rtsp_r_server_response::test_set_header()
{
    r_server_response resA;
    resA.set_header( "name", "value" );
    string value;
    RTF_ASSERT( resA.get_header( "name", value ) );
    RTF_ASSERT( value == "value" );
}

void test_r_rtsp_r_server_response::test_set_body()
{
    r_server_response res;
    res.set_body( "your_mom" );
    RTF_ASSERT( res.get_body_as_string() == "your_mom" );
}

#include <algorithm>

bool is_odd( int i ) { return ((i%2)==1); }

void test_r_rtsp_r_server_response::test_write_response()
{
    int port = RTF_NEXT_PORT();

    thread t([&](){
        usleep(1000000);
        r_socket sok;
        sok.connect( "127.0.0.1", port );

        r_server_response res;
        res.set_header( "boo", "ya" );
        res.set_body( "optimal d2f ratio" );

        res.write_response( sok );
    });
    t.detach();

    r_socket serverSok;
    serverSok.bind( port, ip4_addr_any );
    serverSok.listen();
    auto connected = serverSok.accept();
    r_client_response cres;
    cres.read_response( connected );

    string value;
    RTF_ASSERT( cres.get_header( "boo", value ) );
    RTF_ASSERT( value == "ya" );
    RTF_ASSERT( cres.get_body_as_string() == "optimal d2f ratio" );
}
