
#include "r_rtsp/r_client_connection.h"
#include "r_utils/r_socket.h"
#include "r_utils/r_string_utils.h"

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

r_client_connection::r_client_connection( const std::string& serverIP, int port ) :
    _serverIP( serverIP ),
    _serverPort( port ),
    _sok(),
    _sequence( 0 ),
    _sessionID(),
    _ioTimeOut( 5000 )
{
}

r_client_connection::~r_client_connection() throw()
{
}

void r_client_connection::connect()
{
    _sok.connect( _serverIP, _serverPort );
}

void r_client_connection::disconnect()
{
    _sequence = 0;
    _sessionID = "";
    _sok.close();
}

void r_client_connection::write_request( shared_ptr<r_client_request > request )
{
    _sequence++;

    request->set_server_ip( _serverIP );
    request->set_server_port( _serverPort );

    request->set_header( "CSeq", r_string_utils::int_to_s( _sequence ) );

    if( _sessionID.length() > 0 )
        request->set_header( "Session", _sessionID );

    request->write_request( _sok );
}

shared_ptr<r_client_response> r_client_connection::read_response()
{
    shared_ptr<r_client_response> response = make_shared<r_client_response>();

    response->read_response( _sok );

    string value;
    if( response->get_header( "Session", value ) )
    {
        if( _sessionID.empty() )
            _sessionID = value.substr( 0, value.find(';') ); // If no ';' found, entire string is taken.
    }

    return response;
}

std::string r_client_connection::get_session_id() const
{
    return _sessionID;
}

void r_client_connection::set_session_id( const std::string& sessionID )
{
    _sessionID = sessionID;
}

bool r_client_connection::receive_interleaved_packet( uint8_t& channel, vector<uint8_t>& buffer )
{
    uint8_t dollarSign = 0;
    uint16_t length = 0;
    uint16_t maxIterations = 10000;

    while( maxIterations > 0 )
    {
        maxIterations--;

        _sok.recv( &dollarSign, 1 ); 

        if( dollarSign != 36 )
        {
            R_LOG_WARNING( "r_client_connection [%s] invalid token: %d", _serverIP.c_str(), dollarSign );
            continue;
        }

        _sok.recv( &channel, 1 ); 

        if( channel > 2 )
        {
            R_LOG_WARNING( "r_client_connections [%s] invalid channel: %d", _serverIP.c_str(), channel );
            continue;
        }

        uint16_t sizeShort;
        _sok.recv( &sizeShort, 2 ); 

        length = r_networking::r_ntohs( sizeShort );

        //Interleaved appears to use the same RTP packets as UDP - given that,the max packet size of UDP would be 1500 bytes,
        //we really shouldn't see anything greater than 1600
        if( length > 1600 || length == 0 )
        {
            R_LOG_WARNING( "r_client_connection [%s] invalid size: %d", _serverIP.c_str(), length );
            continue;
        }

        buffer.resize(length);
        _sok.recv( &buffer[0], length );

        return true;
    }

    R_LOG_ERROR("r_client_connection [%s] Unable to sync with stream.", _serverIP.c_str() );

    return false;
}
