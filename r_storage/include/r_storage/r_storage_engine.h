
#ifndef r_storage_r_storage_h
#define r_storage_r_storage_h

#include <string>
#include <vector>

// XXX TODO
// - Determine at startup whether we need to r_storage_engine::create()
//   - index file exist?
//   - iterate filesystems and make sure every file is in index (by last time in file).
//     - if a file is not in the index, add it as the oldest invalid file.

namespace r_storage
{

struct r_file_system
{
    std::string name;
    std::string path;
    uint64_t reserve_bytes;
    uint64_t size_bytes;
    uint64_t free_bytes;
};

struct r_engine_config
{
    uint64_t file_allocation_bytes;
    uint32_t max_files_in_dir;
    uint32_t max_indexes_per_file;
    std::string index_path;
    std::vector<r_file_system> file_systems;
};

class r_storage_engine
{
public:
    static void configure_storage(const std::string& config);
};

}

#endif
