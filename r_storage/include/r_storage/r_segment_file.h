
#ifndef r_storage_r_segment_file_h
#define r_storage_r_segment_file_h

#include <chrono>
#include <string>
#include <vector>
#include <map>

namespace r_storage
{

struct segment_file
{
    std::string id;
    bool valid;
    std::string path;
    uint64_t start_time;
    uint64_t end_time;
    std::string data_source_id;
    std::string type;
    std::string sdp;
};

}

#endif
