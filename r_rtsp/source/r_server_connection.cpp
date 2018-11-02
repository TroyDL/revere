
#include "r_rtsp/r_server_connection.h"
#include "r_rtsp/r_rtsp_server.h"
#include "r_rtsp/r_exception.h"

#include <exception>

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

r_server_connection::r_server_connection( r_rtsp_server* server, r_socket clientSocket ) :
    _thread(),
    _clientSocket( std::move(clientSocket) ),
    _server( server ),
    _sessionID(),
    _myrunning( false ),
    _isStarted( false )
{
}

r_server_connection::~r_server_connection() throw()
{
    if ( _isStarted )
        stop();
}

void r_server_connection::start()
{
    _myrunning = true;
    _thread = std::thread( &r_server_connection::_entry_point, this );
    _isStarted = true;
}

void r_server_connection::stop()
{
    _myrunning = false;
    _thread.join();
}

void r_server_connection::_entry_point()
{
    _myrunning = true;

    while( _myrunning )
    {
        try
        {
            if( !_clientSocket.valid() )
            {
                R_LOG_NOTICE("[r_server_connection|%s] client socket not valid shutting down",_sessionID.c_str());
                _myrunning = false;
                continue;
            }

            uint64_t waitMillis = 2000;
            if( _clientSocket.wait_till_recv_wont_block(waitMillis) )
            {
                // we need buffered_recv() equivalent here. If the recv_wont_block() pops out because the other side
                // closed their end, we need a way to determine if their is actual data in our buffer without having
                // to throw due to a timed out recv().

                shared_ptr<r_server_request> request = make_shared<r_server_request>();
                request->read_request( _clientSocket );

                if( !_clientSocket.valid() )
                {
                    R_LOG_NOTICE("[r_server_connection|%s] client socket not valid shutting down",_sessionID.c_str());
                    _myrunning = false;
                    continue;
                }

                string peerIP = _clientSocket.get_peer_ip();
                request->set_peer_ip( peerIP );
                request->set_local_ip( _clientSocket.get_local_ip() );

                string sessionHeader;
                bool hasSessionHeader = request->get_header( "Session", sessionHeader );

                if( hasSessionHeader && (sessionHeader != _sessionID) )
                    throw_rtsp_exception( 400, "Invalid session header." );

                method m = request->get_method();

                if( m == M_SETUP )
                {
                    if( _sessionID.empty() )
                        _sessionID = r_rtsp_server::get_next_session_id();

                    request->set_header( "Session", _sessionID );
                }
                else if( m == M_TEARDOWN )
                {
                    _myrunning = false;
                }

                string sequenceHeader;
                bool hasSequenceHeader = request->get_header( "CSeq", sequenceHeader );

                if( !hasSequenceHeader )
                    throw_rtsp_exception( 400, "[r_server_connection|%s]Got request without sequence header.", _sessionID.c_str() );

                shared_ptr<r_server_response> response = _server->route_request( request );

                if( ((response->get_status() < STATUS_OK) || (response->get_status() > STATUS_MULTIPLE_CHOICES)) )
                    _sessionID.clear();

                response->set_header( "CSeq", sequenceHeader );

                if( _sessionID.length() && ((response->get_status() >= STATUS_OK) && (response->get_status() < STATUS_MULTIPLE_CHOICES)) )
                    response->set_header( "Session", _sessionID );

                response->write_response( _clientSocket );
                if( !_clientSocket.valid() )
                {
                    R_LOG_NOTICE("[r_server_connection|%s] client socket not valid shutting down",_sessionID.c_str());
                    _myrunning = false;
                    continue;
                }
            }
        }
        catch( exception& ex )
        {
            _myrunning = false;
            R_LOG_ERROR( "[r_server_connection|%s] thread caught exception: %s",_sessionID.c_str(),ex.what() );
        }
    }
}

bool r_server_connection::running() const
{
    return _myrunning;
}

void r_server_connection::write_interleaved_packet( uint8_t channel, const vector<uint8_t>& buffer )
{
    uint8_t token = 36;
    _clientSocket.send( &token, 1 );

    _clientSocket.send( &channel, 1 );

    int16_t length = (int16_t)buffer.size();
    int16_t lengthWord = r_networking::r_htons( length );

    _clientSocket.send( &lengthWord, 2 );

    _clientSocket.send( &buffer[0], buffer.size() );
}
