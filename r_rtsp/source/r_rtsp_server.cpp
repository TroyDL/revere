
#include "r_rtsp/r_rtsp_server.h"
#include "r_rtsp/r_exception.h"

#include <exception>
#include <algorithm>

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

uint32_t r_rtsp_server::_next_session_id = 42;
recursive_mutex r_rtsp_server::_session_id_lock;

r_rtsp_server::r_rtsp_server( const string& serverIP, int port ) :
    _thread(),
    _port( port ),
    _running( false ),
    _sessionPrototypes(),
    _sessions(),
    _connections(),
    _serverSocket( make_shared<r_socket>() ),
    _serverIP( serverIP ),
    _sessionsLock(),
    _connectionsLock(),
    _prototypesLock(),
    _aliveCheck( 1000, std::bind( &r_rtsp_server::check_inactive_sessions, this ) )
{
    _serverSocket->bind( _port, _serverIP );
    _serverSocket->listen();
}

r_rtsp_server::~r_rtsp_server() throw()
{
    if( _running )
        stop();
}

void r_rtsp_server::attach_session_prototype( shared_ptr<r_session_base> session )
{
    unique_lock<recursive_mutex> guard( _prototypesLock );

    _sessionPrototypes.push_back( session );
}

void r_rtsp_server::start()
{
    _running = true;

    {
        unique_lock<recursive_mutex> guard( _prototypesLock );
        if( _sessionPrototypes.empty() )
            R_STHROW( r_rtsp_exception, ( "Please call AttachSessionPrototype() at least once before starting this server." ));
    }

   _thread = std::thread( &r_rtsp_server::_entry_point, this );

   _aliveCheck.start();
}

void r_rtsp_server::stop_session( string id )
{
    {
        unique_lock<recursive_mutex> guard( _sessionsLock );

        auto found = _sessions.find( id );
        if( found != _sessions.end() )
            _sessions.erase( found );
    }

    {
        unique_lock<recursive_mutex> guard( _connectionsLock );

        _connections.erase( remove_if( _connections.begin(),
                                       _connections.end(),
                                       [&](shared_ptr<r_server_connection> i)->bool{ return (i->get_session_id() == id); } ),
                            _connections.end() );
    }
}

void r_rtsp_server::check_inactive_sessions()
{
    unique_lock<recursive_mutex> guard( _sessionsLock );

    for( auto i : _sessions )
    {
        if( i.second->get_timeout_interval_seconds() == 0 )
            continue;

        chrono::steady_clock::time_point then = i.second->get_last_keep_alive_time();
        chrono::steady_clock::time_point now = chrono::steady_clock::now();

        uint64_t deltaSeconds = chrono::duration_cast<std::chrono::seconds>( now - then ).count();

        if( deltaSeconds < i.second->get_timeout_interval_seconds() )
            continue;

        stop_session( i.second->get_session_id() );
    }
}

void r_rtsp_server::stop()
{
    _running = false;

    if( _serverSocket->valid() )
        _serverSocket->close();

    _thread.join();
    _aliveCheck.stop();

    {
        // This joins ALL our ServerConnection threads.
        unique_lock<recursive_mutex> guard( _connectionsLock );
        _connections.clear();
    }
}

void* r_rtsp_server::_entry_point()
{
    while( _running )
    {
        try
        {
            uint64_t timeoutMillis = 250;
            auto recvWontBlock = _serverSocket->wait_till_recv_wont_block( timeoutMillis );

            if( !_serverSocket->valid() )
                _running = false;

            if( !_running )
                continue;

            if(recvWontBlock)
            {
                shared_ptr<r_server_connection> serverConnection = make_shared<r_server_connection>( this, _serverSocket->accept() );

                {
                    unique_lock<recursive_mutex> guard( _connectionsLock );

                    _connections.push_back( serverConnection );

                    serverConnection->start();
                }
            }

            {
                unique_lock<recursive_mutex> guard( _connectionsLock );
                _connections.erase( remove_if( _connections.begin(),
                                               _connections.end(),
                                               [](shared_ptr<r_server_connection> i)->bool{ return !i->running(); } ),
                                    _connections.end() );
            }
        }
        catch( std::exception& ex )
        {
            R_LOG_ERROR( "[r_rtsp_server] EntryPoint Exception: %s", ex.what() );
        }
    }

    return nullptr;
}

shared_ptr<r_server_response> r_rtsp_server::route_request( shared_ptr<r_server_request> request )
{
    method m = request->get_method();

    if( m == M_OPTIONS )
        return _handle_options( request );

    if( m == M_DESCRIBE )
        return _handle_describe( request );

    string sessionHeader;
    bool hasSessionHeader = request->get_header( "Session", sessionHeader );

    if( !hasSessionHeader )
        R_STHROW( r_rtsp_exception, ( "Session header not found." ));

    if( m == M_SETUP )
        _handle_setup( request );

    shared_ptr<r_session_base> session;

    {
        unique_lock<recursive_mutex> guard( _sessionsLock );
        auto found = _sessions.find( sessionHeader );
        if( found != _sessions.end() )
            session = found->second;
    }

    if( session.get() == nullptr )
        R_STHROW( r_rtsp_exception, ( "No session was found." ));

    if ( m == M_GET_PARAM )
        session->update_keep_alive_time();

    shared_ptr<r_server_response> response = session->handle_request( request );

    if( m == M_TEARDOWN )
    {
        unique_lock<recursive_mutex> guard( _sessionsLock );
        _sessions.erase( sessionHeader );
    }

    return response;
}

string r_rtsp_server::get_next_session_id()
{
    unique_lock<recursive_mutex> guard( _session_id_lock );

    string ID = r_string::uint32_to_s( _next_session_id );

    _next_session_id++;

    return ID;
}

shared_ptr<r_server_response> r_rtsp_server::_handle_options( shared_ptr<r_server_request> request )
{
    unique_lock<recursive_mutex> guard( _prototypesLock );
    if(_sessionPrototypes.empty())
        R_THROW(("No added session prototypes!"));

    shared_ptr<r_server_response> response = make_shared<r_server_response>();

    response->set_header( "Public", _sessionPrototypes.front()->get_supported_options() );

    return response;
}

shared_ptr<r_server_response> r_rtsp_server::_handle_describe( shared_ptr<r_server_request> request )
{
    string presentation = r_string::strip(request->get_uri());

    shared_ptr<r_session_base> prototype = _locate_session_prototype( presentation );

    shared_ptr<r_session_base> session = prototype->clone();
    shared_ptr<r_server_response> response = session->handle_request( request );

    response->set_header("Content-type","application/sdp");
    response->set_header("Content-Base", r_string::format( "rtsp://%s:%d/%s/",
                                                           _serverIP.c_str(),
                                                           _port,
                                                           presentation.c_str()) );

    return response;
}

void r_rtsp_server::_handle_setup( shared_ptr<r_server_request> request )
{
    string sessionHeader;
    if( !request->get_header( "Session", sessionHeader ) )
        R_STHROW( r_rtsp_exception, ( "Session header not found." ));

    string presentation = r_string::strip(request->get_uri());

    shared_ptr<r_session_base> prototype = _locate_session_prototype( presentation );

    if( prototype.get() == nullptr )
        R_STHROW( r_rtsp_exception, ( "Suitable connection prototype for this presentation not found." ));

    shared_ptr<r_session_base> session = prototype->clone();
    session->update_keep_alive_time();

    session->set_session_id( sessionHeader );

    {
        unique_lock<recursive_mutex> guard( _sessionsLock );
        _sessions.insert( make_pair( sessionHeader, session ) );
    }
}

shared_ptr<r_session_base> r_rtsp_server::_locate_session_prototype( const string& resourcePath )
{
    unique_lock<recursive_mutex> guard( _prototypesLock );
    for( auto i : _sessionPrototypes )
    {
        if( i->handles_this_presentation( resourcePath ) )
            return i;
    }

    R_STHROW( r_rtsp_exception, ( "Suitable connection prototype for this presentation not found." ) );
}
