
#ifndef _r_vss_client_r_sdp_h
#define _r_vss_client_r_sdp_h

#include <string>
#include <chrono>
#include <vector>

namespace r_vss_client
{

std::string fetch_sdp_before(const std::string& dataSourceID,
                             const std::string& type,
                             const std::chrono::system_clock::time_point& time);

}

#endif