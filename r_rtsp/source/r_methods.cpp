
#include "r_rtsp/r_methods.h"
#include "r_rtsp/r_exception.h"
#include "r_utils/r_string.h"

using namespace r_rtsp;
using namespace r_utils;
using namespace std;

static const string method_names[] = { "OPTIONS",
                                          "DESCRIBE",
                                          "SETUP",
                                          "TEARDOWN",
                                          "PLAY",
                                          "PAUSE",
                                          "GET_PARAMETER",
                                          "SET_PARAMETER",
                                          "REDIRECT" };

string r_rtsp::get_method_name( method m )
{
    if( m >= M_NUM_METHODS )
        throw_rtsp_exception( 500, "Unknown method." );

    return method_names[m];
}

#ifdef IS_WINDOWS
#pragma warning( disable : 4715 )
#endif

method r_rtsp::get_method( const std::string& methodName )
{
    if( methodName == "OPTIONS" )
    {
        return M_OPTIONS;
    }
    else if( methodName == "DESCRIBE" )
    {
        return M_DESCRIBE;
    }
    else if( methodName == "SETUP" )
    {
        return M_SETUP;
    }
    else if( methodName == "TEARDOWN" )
    {
        return M_TEARDOWN;
    }
    else if( methodName == "PLAY" )
    {
        return M_PLAY;
    }
    else if( methodName == "PAUSE" )
    {
        return M_PAUSE;
    }
    else if( methodName == "GET_PARAMETER" )
    {
        return M_GET_PARAM;
    }
    else if( methodName == "SET_PARAMETER" )
    {
        return M_SET_PARAM;
    }
    else if( methodName == "REDIRECT" )
    {
        return M_REDIRECT;
    }

    throw_rtsp_exception( 400, "Unknown method." );

    R_THROW(("Impossible Exception!")); // this should never happen, but it fixes a warning...
}
