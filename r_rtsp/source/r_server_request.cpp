
#include "r_rtsp/r_server_request.h"
#include "r_rtsp/r_exception.h"

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

r_server_request::r_server_request() :
    _initialLine(),
    _requestLines(),
    _headerParts(),
    _peerIP(),
    _localIP()
{
}

r_server_request::r_server_request( const r_server_request& rhs ) :
    _initialLine( rhs._initialLine ),
    _requestLines( rhs._requestLines ),
    _headerParts( rhs._headerParts ),
    _peerIP( rhs._peerIP ),
    _localIP( rhs._localIP )
{
}

r_server_request::r_server_request( r_server_request&& rhs ) throw() :
    _initialLine( std::move( rhs._initialLine ) ),
    _requestLines( std::move( rhs._requestLines ) ),
    _headerParts( std::move( rhs._headerParts ) ),
    _peerIP( std::move( rhs._peerIP ) ),
    _localIP( std::move( rhs._localIP ) )
{
}

r_server_request::~r_server_request() throw()
{
}

r_server_request& r_server_request::operator = ( const r_server_request& rhs )
{
    _initialLine = rhs._initialLine;
    _requestLines = rhs._requestLines;
    _headerParts = rhs._headerParts;
    _peerIP = rhs._peerIP;
    _localIP = rhs._localIP;

    return *this;
}

r_server_request& r_server_request::operator = ( r_server_request&& rhs ) throw()
{
    _initialLine = std::move( rhs._initialLine );
    _requestLines = std::move( rhs._requestLines );
    _headerParts = std::move( rhs._headerParts );
    _peerIP = std::move( rhs._peerIP );
    _localIP = std::move( rhs._localIP );

    return *this;
}

method r_server_request::get_method() const
{
    auto found = _headerParts.find( "method" );
    if( found != _headerParts.end() )
        return r_rtsp::get_method( found->second );

    R_STHROW( rtsp_400_exception, ( "Unknown method." ));
}

string r_server_request::get_url() const
{
    auto found = _headerParts.find( "url" );
    if( found != _headerParts.end() )
        return found->second;

    R_STHROW( rtsp_400_exception, ( "URI not found." ));
}

string r_server_request::get_uri() const
{
    auto found = _headerParts.find( "uri" );
    if( found != _headerParts.end() )
        return found->second;

    R_STHROW( rtsp_400_exception, ( "resource path not found." ));
}

void r_server_request::set_peer_ip( const std::string& peerIP )
{
    _peerIP = peerIP;
}

string r_server_request::get_peer_ip() const
{
    return _peerIP;
}

void r_server_request::set_local_ip( const std::string& localIP )
{
    _localIP = localIP;
}

string r_server_request::get_local_ip() const
{
    return _localIP;
}

bool r_server_request::get_header( const string& key, string& value ) const
{
    auto found = _headerParts.find( key );
    if( found != _headerParts.end() )
    {
        value = found->second;
        return true;
    }

    return false;
}

void r_server_request::set_header( const std::string& key, const std::string& value )
{
    _headerParts.insert( std::make_pair( key, value ) );
}

void r_server_request::read_request( r_utils::r_stream_io& sok )
{
    /// First, read the initial request line...
    string temp = "";
    char lineBuf[MAX_HEADER_LINE+1];
    memset( lineBuf, 0, MAX_HEADER_LINE+1 );

    char* writer = &lineBuf[0];

    bool lineDone = false;

    size_t bytesReadThisLine = 0;

    while( !lineDone && ((bytesReadThisLine+1) < MAX_HEADER_LINE) )
    {
        sok.recv( writer, 1 );

        bytesReadThisLine += 1;

        if( *writer == '\n' )
            lineDone = true;

        writer++;
    }

    if( !lineDone )
        R_STHROW( rtsp_400_exception, ( "The RTSP initial request line exceeded our max." ) );

    _initialLine = lineBuf;
    temp += _initialLine;
    /// Now, read the rest of the header lines...

    bool headerDone = false;
    while( !headerDone )
    {
        memset( lineBuf, 0, MAX_HEADER_LINE );

        writer = &lineBuf[0];

        bytesReadThisLine = 0;

        lineDone = false;

        while( !lineDone && ((bytesReadThisLine+1) < MAX_HEADER_LINE) )
        {
            sok.recv( writer, 1 );

            bytesReadThisLine += 1;

            if( *writer == '\n' )
                lineDone = true;

            writer++;
        }

        if( !lineDone )
            R_STHROW( rtsp_400_exception, ( "The RTSP initial request line exceeded our max." ) );

        if( (lineBuf[0] == '\r' && lineBuf[1] == '\n') || (lineBuf[0] == '\n') )
            headerDone = true;
        else
        {
            const string line = lineBuf;
            _parse_additional_lines(line);
        }
        temp += lineBuf;
    }

    /// Now, populate our header hash...
    _parse_initial_line(_initialLine);
}

void r_server_request::_parse_additional_lines( const string& str )
{
    if( r_string_utils::starts_with(str, " ") || r_string_utils::starts_with(str, "\t") )
    {
        auto last = _requestLines.rbegin();
        if( last != _requestLines.rend() )
            *last += str;
        else R_STHROW( rtsp_400_exception, ( "First line of header missing needed seperator." ));
    }
    else _requestLines.push_back( str );
}

void r_server_request::_parse_initial_line( string& str )
{
    _headerParts.clear();

    const vector<string> initialLineParts = r_string_utils::split(str, ' ');

    if( initialLineParts.size() != 3 )
    {
        R_STHROW( rtsp_400_exception, ( "RTSP initial line not composed of 3 parts." ));
    }

    _headerParts.insert( std::make_pair( "method", initialLineParts[0] ) );
    _headerParts.insert( std::make_pair( "url", initialLineParts[1] ) );

    static const string protocolString("://");

    size_t index = initialLineParts[1].find(protocolString);
    if ( index == std::string::npos )
        R_STHROW( rtsp_400_exception, ("RTSP URIs must have a protocol type") );

    index = initialLineParts[1].find("/",index+protocolString.length());

    if ( index != string::npos && index+1 < initialLineParts[1].length() )
        _headerParts.insert( make_pair( "uri", initialLineParts[1].substr(index) ) );
    else _headerParts.insert( make_pair( "uri", string("/") ) );

    _headerParts.insert( make_pair( "rtsp_version", initialLineParts[2] ) );

    for( auto line : _requestLines )
    {
        const vector<string> lineParts = r_string_utils::split(line, ':');
        _headerParts.insert( std::make_pair( lineParts[0], r_string_utils::strip(lineParts[1]) ) );
    }
}
