
#include "r_rtsp/r_server_response.h"
#include "r_rtsp/r_exception.h"
#include <time.h>

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

r_server_response::r_server_response() :
    _status( STATUS_OK ),
    _headers(),
    _body()
{
}

r_server_response::r_server_response( const r_server_response& rhs ) :
    _status( rhs._status ),
    _headers( rhs._headers ),
    _body( rhs._body )
{
}

r_server_response::r_server_response( r_server_response&& rhs ) throw() :
    _status( std::move( rhs._status ) ),
    _headers( std::move( rhs._headers ) ),
    _body( std::move( rhs._body ) )
{
}

r_server_response::~r_server_response() throw()
{
}


r_server_response& r_server_response::operator = ( const r_server_response& rhs )
{
    _status = rhs._status;
    _headers = rhs._headers;
    _body = rhs._body;

    return *this;
}

r_server_response& r_server_response::operator = ( r_server_response&& rhs ) throw()
{
    _status = std::move( rhs._status );
    _headers = std::move( rhs._headers );
    _body = std::move( rhs._body );

    return *this;
}

void r_server_response::set_status( status s )
{
    _status = s;
}

status r_server_response::get_status() const
{
    return _status;
}

void r_server_response::set_header( const string& key, const string& value )
{
    _headers.insert( make_pair( key, value ) );
}

bool r_server_response::get_header( const string& key, string& value )
{
    auto found = _headers.find( key );
    if( found != _headers.end() )
    {
        value = found->second;
        return true;
    }

    return false;
}

void r_server_response::set_body( const vector<uint8_t>& body )
{
    _body = body;
}

vector<uint8_t> r_server_response::get_body() const
{
    return _body;
}

void r_server_response::set_body( const std::string& body )
{
    auto len = body.length();
    _body.resize(len);
    memcpy(&_body[0], body.c_str(), len);
}

string r_server_response::get_body_as_string() const
{
    return string( (char*)&_body[0], _body.size() );
}

void r_server_response::write_response( r_stream_io& sok )
{
    string messageHeader = r_string_utils::format( "RTSP/1.0 %d %s\r\n", _status, get_status_message( _status ).c_str() );

    // Please Note: This is NOT just getting the time, it is strategically chomping a
    // very particular character!
    time_t now = time( 0 );
    char* timeString = ctime( &now );
    size_t timeLen = strlen( timeString );
    timeString[timeLen-1] = 0;

    string dateTime = timeString;

    string dateLine = r_string_utils::format( "Date: %s\r\n", dateTime.c_str() );

    for( auto i : _headers )
        messageHeader += r_string_utils::format( "%s: %s\r\n", i.first.c_str(), i.second.c_str() );

    size_t bodySize = _body.size();

    if( bodySize > 0 )
        messageHeader += r_string_utils::format( "Content-Length: %d\r\n", bodySize );

    messageHeader += r_string_utils::format( "\r\n" );

    sok.send( messageHeader.c_str(), messageHeader.length() );

    if( bodySize )
        sok.send( &_body[0], bodySize );
}

r_server_response r_server_response::create_response_to( method m )
{
    r_server_response ssr;

    if( m == M_OPTIONS )
    {
        ssr.set_header( "Public", "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER, REDIRECT, RECORD" );
    }

    return ssr;
}
