

#ifndef r_storage_r_queries_h
#define r_storage_r_queries_h

#include <chrono>
#include <string>
#include <vector>

namespace r_storage
{

std::string contents(const std::string& indexPath,
                     const std::string& dataSourceID,
                     const std::chrono::system_clock::time_point& start,
                     const std::chrono::system_clock::time_point& end,
                     bool utc);

std::vector<uint8_t> key_before(const std::string& indexPath,
                                const std::string& dataSourceID,
                                const std::chrono::system_clock::time_point& time);

std::vector<uint8_t> query(const std::string& indexPath,
                           const std::string& dataSourceID,
                           const std::string& type,
                           bool previousPlayable,
                           const std::chrono::system_clock::time_point& start,
                           const std::chrono::system_clock::time_point& end);

std::string sdp_before(const std::string& indexPath,
                       const std::string& dataSourceID,
                       const std::string& type,
                       const std::chrono::system_clock::time_point& time);

}

#endif
