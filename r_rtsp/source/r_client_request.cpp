
#include "r_rtsp/r_client_request.h"
#include "r_rtsp/r_exception.h"

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

r_client_request::r_client_request ( method m ) :
    _method( m ),
    _userAgent( "r_rtsp version 1.0.0" ),
    _additionalHeaders(),
    _serverIP(),
    _serverPort(),
    _uri()
{
}

r_client_request::r_client_request ( const r_client_request & rhs ) :
    _method( rhs._method ),
    _userAgent( rhs._userAgent ),
    _additionalHeaders( rhs._additionalHeaders ),
    _serverIP( rhs._serverIP ),
    _serverPort( rhs._serverPort ),
    _uri( rhs._uri )
{
}

r_client_request::r_client_request( r_client_request&& rhs ) throw() :
    _method( std::move( rhs._method ) ),
    _userAgent( std::move( rhs._userAgent ) ),
    _additionalHeaders( std::move( rhs._additionalHeaders ) ),
    _serverIP( std::move( rhs._serverIP ) ),
    _serverPort( std::move( rhs._serverPort ) ),
    _uri( std::move( rhs._uri ) )
{
}

r_client_request::~r_client_request() throw()
{
}

r_client_request& r_client_request::operator = ( const r_client_request& rhs )
{
    _method = rhs._method;
    _userAgent = rhs._userAgent;
    _additionalHeaders = rhs._additionalHeaders;
    _serverIP = rhs._serverIP;
    _serverPort = rhs._serverPort;
    _uri = rhs._uri;

    return *this;
}

r_client_request& r_client_request::operator = ( r_client_request&& rhs ) throw()
{
    _method = std::move( rhs._method );
    _userAgent = std::move( rhs._userAgent );
    _additionalHeaders = std::move( rhs._additionalHeaders );
    _serverIP = std::move( rhs._serverIP );
    _serverPort = std::move( rhs._serverPort );
    _uri = std::move( rhs._uri );

    return *this;
}

void r_client_request::set_method( method m )
{
    _method = m;
}

method r_client_request::get_method() const
{
    return _method;
}

void r_client_request::set_user_agent( const string& userAgent )
{
    _userAgent = userAgent;
}

string r_client_request::get_user_agent() const
{
    return _userAgent;
}

void r_client_request::set_header( const string& name, const string& value )
{
    _additionalHeaders.insert( make_pair( name, value ) );
}

bool r_client_request::get_header( const string& name, string& value ) const
{
    auto found = _additionalHeaders.find( name );

    if( found != _additionalHeaders.end() )
    {
        value = (*found).second;
        return true;
    }

    return false;
}

void r_client_request::set_server_ip( const string& serverIP )
{
    _serverIP = serverIP;
}

string r_client_request::get_server_ip() const
{
    return _serverIP;
}

void r_client_request::set_server_port( int port )
{
    _serverPort = port;
}

int r_client_request::get_server_port() const
{
    return _serverPort;
}

void r_client_request::set_uri( const string& uri )
{
    if( uri.length() == 0 )
        R_STHROW( rtsp_400_exception, ( "Invalid URI set on r_client_request." ) );

    if( r_string_utils::starts_with( uri, "/" ) )
        _uri = uri.substr( 1 );
    else _uri = uri;
}

string r_client_request::get_uri() const
{
    return r_string_utils::format( "/%s", _uri.c_str() );
}

void r_client_request::write_request( r_stream_io& sok )
{
    string rtspURL = r_string_utils::format( "rtsp://%s:%d/%s",
                                        _serverIP.c_str(),
                                        _serverPort,
                                        _uri.c_str() );

    if( _method < M_OPTIONS || _method > M_REDIRECT )
        R_STHROW( rtsp_400_exception, ( "method not recognized." ));

    string request = get_method_name( _method );

    string initialLine = r_string_utils::format( "%s %s RTSP/1.0\r\n", request.c_str(), rtspURL.c_str() );

    string message = initialLine;

    message += r_string_utils::format( "User-Agent: %s\r\n", _userAgent.c_str() );

    for( auto i : _additionalHeaders )
        message += r_string_utils::format( "%s: %s\r\n", i.first.c_str(), i.second.c_str() );

    message += r_string_utils::format( "\r\n" );

    sok.send( message.c_str(), message.length() );
}
