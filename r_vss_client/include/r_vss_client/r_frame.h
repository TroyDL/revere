
#ifndef _r_vss_client_r_frame_h
#define _r_vss_client_r_frame_h

#include <vector>
#include <string>
#include <chrono>

namespace r_vss_client
{

std::vector<uint8_t> fetch_key_before(const std::string& dataSourceID, const std::chrono::system_clock::time_point& time);

}

#endif