
#include "r_rtsp/r_exception.h"

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

r_rtsp_exception::r_rtsp_exception() :
    r_exception()
{
}

r_rtsp_exception::r_rtsp_exception( const char* msg, ... ) :
    r_exception()
{
    va_list args;
    va_start( args, msg );
    set_msg( r_string_utils::format( msg, args ) );
    va_end( args );
}

r_rtsp_exception::r_rtsp_exception( const string& msg ) :
    r_exception( msg )
{
}

rtsp_exception::rtsp_exception( int statusCode ) :
    r_rtsp_exception(),
    _statusCode( statusCode )
{
}

rtsp_exception::rtsp_exception( int statusCode, const char* msg, ... ) :
    r_rtsp_exception(),
    _statusCode( statusCode )
{
    va_list args;
    va_start( args, msg );
    set_msg( r_string_utils::format( msg, args ) );
    va_end( args );
}

rtsp_exception::rtsp_exception( int statusCode, const string& msg ) :
    r_rtsp_exception( msg ),
    _statusCode( statusCode )
{
}

void r_rtsp::throw_rtsp_exception( int statusCode, const char* msg, ... )
{
    va_list args;
    va_start( args, msg );
    const string message = r_string_utils::format( msg, args );
    va_end( args );

    throw_rtsp_exception( statusCode, message );
}

void r_rtsp::throw_rtsp_exception( int statusCode, const string& msg )
{
    switch( statusCode )
    {
        case 400:
        {
            rtsp_400_exception e( msg );
            throw e;
        }
        case 401:
        {
            rtsp_401_exception e( msg );
            throw e;
        }
        case 403:
        {
            rtsp_403_exception e( msg );
            throw e;
        }
        case 404:
        {
            rtsp_404_exception e( msg );
            throw e;
        }
        case 415:
        {
            rtsp_415_exception e( msg );
            throw e;
        }
        case 453:
        {
            rtsp_453_exception e( msg );
            throw e;
        }
        case 500:
        {
            rtsp_500_exception e( msg );
            throw e;
        }
        default:
        {
            rtsp_exception e( statusCode, msg );
            throw e;
        }
    }
}
