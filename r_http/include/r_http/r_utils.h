#ifndef r_http_r_utils_h
#define r_http_r_utils_h

#include "r_utils/r_string.h"

namespace r_http
{

void parse_url_parts( const std::string url, std::string& host, int& port, std::string& protocol, std::string& uri );

std::string adjust_header_name( const std::string& value );

std::string adjust_header_value( const std::string& value );

}

#endif