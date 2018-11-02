
#ifndef _r_vss_client_r_query_h
#define _r_vss_client_r_query_h

#include <string>
#include <vector>
#include <chrono>

namespace r_vss_client
{

std::vector<uint8_t> query(const std::string& dataSourceID,
                           const std::string& type,
                           bool previousPlayable,
                           const std::chrono::system_clock::time_point& start,
                           const std::chrono::system_clock::time_point& end);

std::vector<uint8_t> query(const std::string& uri);

}

#endif
