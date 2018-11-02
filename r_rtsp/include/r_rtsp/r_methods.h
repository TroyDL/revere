
#ifndef r_rtsp_methods_h
#define r_rtsp_methods_h

#include "r_utils/r_string.h"

namespace r_rtsp
{

enum method
{
    M_OPTIONS,
    M_DESCRIBE,
    M_SETUP,
    M_TEARDOWN,
    M_PLAY,
    M_PAUSE,
    M_GET_PARAM,
    M_SET_PARAM,
    M_REDIRECT,

    M_NUM_METHODS
};

std::string get_method_name( method m );

method get_method( const std::string& methodName );

}

#endif
