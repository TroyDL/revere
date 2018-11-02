
#ifndef _r_vss_client_r_query_generator_h
#define _r_vss_client_r_query_generator_h

#include "r_utils/r_time_utils.h"
#include "r_utils/r_nullable.h"
#include <string>
#include <chrono>

namespace r_vss_client
{

class r_query_generator final
{
public:
    r_query_generator(const std::string& dataSourceID,
                      const std::string& type,
                      uint64_t requestSize,
                      const std::chrono::system_clock::time_point& start,
                      const std::chrono::system_clock::time_point& end);
    r_query_generator(const r_query_generator&) = default;
    r_query_generator(r_query_generator&&) = default;

    ~r_query_generator() noexcept;

    r_query_generator& operator=(const r_query_generator&) = default;
    r_query_generator& operator=(r_query_generator&&) = default;

    r_utils::r_nullable<std::string> next();

private:
    std::string _dataSourceID;
    std::chrono::system_clock::time_point _start;
    std::chrono::system_clock::time_point _end;
    std::chrono::system_clock::time_point _next;
    std::string _type;
    uint64_t _chunkMillis;
    bool _first;
};

}

#endif
