
#include "test_r_rtsp_r_rtsp_server.h"
#include "r_rtsp/r_rtsp_server.h"
#include "r_rtsp/r_client_request.h"
#include "r_rtsp/r_server_request.h"
#include "r_rtsp/r_server_response.h"
#include "r_rtsp/r_client_response.h"
#include "r_rtsp/r_client_connection.h"
#include <memory>

using namespace std;
using namespace r_utils;
using namespace r_rtsp;

REGISTER_TEST_FIXTURE(test_r_rtsp_r_rtsp_server);

class simple_session : public r_session_base
{
public:
    simple_session( r_rtsp_server& server ) : r_session_base( server ) {}
    simple_session( r_rtsp_server& server, uint64_t sessionTimeOutSeconds ) : r_session_base( server, sessionTimeOutSeconds ) {}
    virtual ~simple_session() throw() {}
    virtual shared_ptr<r_session_base> clone() const { return make_shared<simple_session>( _server ); }
    virtual bool handles_this_presentation( const string& presentation ) { return r_string_utils::contains(presentation, "/foo/bar"); }
    virtual string get_supported_options() { return "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN"; }
    virtual shared_ptr<r_server_response> handle_request( shared_ptr<r_server_request> request )
    {
        shared_ptr<r_server_response> response = make_shared<r_server_response>();

        switch( request->get_method() )
        {
        case M_DESCRIBE:
            response->set_body("THIS IS MY SDP");
            break;
        case M_SETUP:
            break;
        case M_PLAY:
            break;
        case M_TEARDOWN:
            break;
        case M_GET_PARAM:
            break;
        default:
            break;
        };

        return response;
    }
private:
};

void test_r_rtsp_r_rtsp_server::setup()
{
}

void test_r_rtsp_r_rtsp_server::teardown()
{
}

void test_r_rtsp_r_rtsp_server::test_constructor()
{
    int port = RTF_NEXT_PORT();
    r_rtsp_server server( ip4_addr_any, port );
}

void test_r_rtsp_r_rtsp_server::test_start_stop()
{
    int port = RTF_NEXT_PORT();

    r_rtsp_server server( ip4_addr_any, port );
    server.attach_session_prototype( make_shared<simple_session>( server ) );
    server.start();
    server.stop();
}

void test_r_rtsp_r_rtsp_server::test_options()
{
    int port = RTF_NEXT_PORT();

    r_rtsp_server server( ip4_addr_any, port );
    server.attach_session_prototype( make_shared<simple_session>( server ) );
    server.start();
    shared_ptr<r_client_request> req = make_shared<r_client_request>( M_DESCRIBE );
    req->set_uri( "/foo/bar" );
    r_client_connection conn( "127.0.0.1", port );
    conn.connect();
    conn.write_request( req );
    shared_ptr<r_client_response> res = conn.read_response();
    RTF_ASSERT( res->get_body_as_string() == "THIS IS MY SDP" );
    RTF_ASSERT( res->get_status() == STATUS_OK );
    server.stop();
}

void test_r_rtsp_r_rtsp_server::test_describe()
{
    int port = RTF_NEXT_PORT();

    r_rtsp_server server( ip4_addr_any, port );
    server.attach_session_prototype( make_shared<simple_session>( server ) );
    server.start();
    shared_ptr<r_client_request> req = make_shared<r_client_request>( M_OPTIONS );
    req->set_uri( "/foo/bar" );
    r_client_connection conn( "127.0.0.1", port );
    conn.connect();
    conn.write_request( req );
    shared_ptr<r_client_response> res = conn.read_response();
    string value;
    RTF_ASSERT( res->get_header( "Public", value ) );
    RTF_ASSERT( value == "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN" );
    RTF_ASSERT( res->get_status() == STATUS_OK );
    server.stop();
}
