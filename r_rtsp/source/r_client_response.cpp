
#include "r_rtsp/r_client_response.h"
#include "r_rtsp/r_exception.h"

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

r_client_response::r_client_response() :
    _headerPieces(),
    _statusCode(),
    _body()
{
}

r_client_response::r_client_response( const r_client_response& rhs ) :
    _headerPieces( rhs._headerPieces ),
    _statusCode( rhs._statusCode ),
    _body( rhs._body )
{
}

r_client_response::r_client_response( r_client_response&& rhs ) throw() :
    _headerPieces( std::move( rhs._headerPieces ) ),
    _statusCode( std::move( rhs._statusCode ) ),
    _body( std::move( rhs._body ) )
{
}

r_client_response::~r_client_response() throw()
{
}

r_client_response& r_client_response::operator = ( const r_client_response& obj )
{
    _headerPieces = obj._headerPieces;
    _statusCode = obj._statusCode;
    _body = obj._body;

    return *this;
}

r_client_response& r_client_response::operator = ( r_client_response&& obj ) throw()
{
    _headerPieces = std::move( obj._headerPieces );
    _statusCode = std::move( obj._statusCode );
    _body = std::move( obj._body );

    return *this;
}

bool r_client_response::get_header( const string& key, string& value ) const
{
    auto found = _headerPieces.find( key );

    if( found != _headerPieces.end() )
    {
        value = (*found).second;
        return true;
    }

    return false;
}

vector<uint8_t> r_client_response::get_body() const
{
    return _body;
}

string r_client_response::get_body_as_string() const
{
    return string( (char*)&_body[0], _body.size());
}

void r_client_response::read_response( r_stream_io& sok )
{
    do
    {
        bool messageEnd = false;
        std::vector<string> lines;
        size_t bodySize = 0;

        do
        {
            string line;
            int recvCount = 0;
            int foundCount = 0;
            bool foundCarriageReturn = false;

            do
            {
                char character = 0;
                sok.recv( &character, 1 );
                ++recvCount;

                if ( character == '\n' )
                {
                    lines.push_back(line);
                    break;
                }

                if( character != '\r' )
                {
                    if ( foundCarriageReturn )
                        R_STHROW(r_rtsp_exception, ("Failed to find a line feed and have found a Carriage Return before current character(%c)",character));
                    line += character;
                    ++foundCount;
                }
                else foundCarriageReturn = true;

            } while(recvCount>0);

            if ( recvCount == 0 && foundCount != 0 )
                R_STHROW(r_rtsp_exception, ("Failed to find end line but not finding data"));

            messageEnd = foundCount == 0;

        }while(!messageEnd);

        bodySize = _parse_lines( lines );

        if ( bodySize > 0 )
            _receive_body(sok, bodySize);

    }while(_statusCode == STATUS_CONTINUE);
}

status r_client_response::get_status() const
{
    return _statusCode;
}

string r_client_response::get_session() const
{
    string value;
    if( get_header("Session",value) )
        return value;
    R_STHROW(r_rtsp_exception, ("There is no Session Key!"));
}

vector<string> r_client_response::get_methods() const
{
    string value;
    if( get_header("Public",value) )
    {
        std::vector<string> methods = r_string::split(value, ',');
        for( size_t i=0; i < methods.size(); ++i )
            methods[i] = r_string::strip(methods[i]);;
        return methods;
    }
    R_STHROW(r_rtsp_exception, ("No \"Public\" method found!"));
}

void r_client_response::_receive_body( r_stream_io& sok, size_t bodySize )
{
    _body.resize(bodySize);
    sok.recv( &_body[0], bodySize );
}

size_t r_client_response::_parse_lines( vector<string>& lines )
{
    size_t result = 0;

    for ( size_t i = 0; i < lines.size(); ++i )
    {
        if ( r_string::contains(lines[i], "RTSP") )
        {
            std::vector<string> parts = r_string::split(lines[i], ' ');

            if ( parts.size() < 2 )
                R_STHROW(r_rtsp_exception, ("Not enough parts in status line."));

            _statusCode = convert_status_code_from_int(r_string::s_to_int(parts[1]));

            continue;
        }

        std::vector<string> parts = r_string::split(lines[i], ": ");

        if ( lines[i].empty() )
            continue;

        if ( parts.size() != 2 )
            R_STHROW(r_rtsp_exception, ("Wrong number of parts"));

        if ( r_string::contains(lines[i], "Content-Length") )
            result = r_string::s_to_int(parts[1]);

        _headerPieces.insert(make_pair(parts[0],parts[1]));
    }

    return result;
}
