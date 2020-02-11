
#include "r_storage/r_storage_engine.h"
#include "r_storage/r_append_file.h"
#include "r_storage/r_segment_file.h"
#include "r_storage/r_file_index.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_file.h"
#include "r_utils/r_uuid.h"
#include "r_utils/3rdparty/json/json.h"
#include "r_db/r_sqlite_conn.h"
#include <chrono>
#include <algorithm>

using namespace r_storage;
using namespace r_utils;
using namespace r_db;
using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

static r_file_system _parse_file_system(const string& fs)
{
    auto fsj = json::parse(fs);
    r_file_system rfs;
    rfs.name = fsj["name"].get<string>();
    rfs.path = fsj["path"].get<string>();
    rfs.reserve_bytes = r_string_utils::s_to_uint64(fsj["reserve_bytes"].get<string>());

    r_fs::get_fs_usage(rfs.path, rfs.size_bytes, rfs.free_bytes);

    if(rfs.reserve_bytes > rfs.size_bytes)
        R_STHROW(r_invalid_argument_exception, ("reserve_bytes must be smaller than the file system size!"));

    return rfs;
}

void _fill_file_systems(const r_engine_config& cfg)
{
    vector<string> paths;

    for(auto fs : cfg.file_systems)
    {
        //uint64_t size_bytes, free_bytes;
        //r_fs::get_fs_usage(fs.path, size_bytes, free_bytes);

        uint64_t bytesToFill = (fs.free_bytes > fs.reserve_bytes)?fs.free_bytes - fs.reserve_bytes:0;

        if(bytesToFill < cfg.file_allocation_bytes)
	        continue;

        auto numFiles = (uint32_t)(bytesToFill / cfg.file_allocation_bytes);

        printf("numFiles = %d\n",numFiles);
        fflush(stdout);

        auto remainderFiles = numFiles % cfg.max_files_in_dir;

        uint32_t numDirs = 1;
        if(numFiles > cfg.max_files_in_dir)
        {
            numDirs = numFiles / cfg.max_files_in_dir;
            if(remainderFiles > 0)
	            ++numDirs;
        }

        if(numFiles > 0)
        {
            for(uint32_t i = 0; i < numDirs; ++i)
            {
                auto dir = fs.path + r_fs::PATH_SLASH + r_uuid::generate();

                r_fs::mkdir(dir);

                auto filesToCreate = (remainderFiles>0)?(i==0)?remainderFiles:cfg.max_files_in_dir:cfg.max_files_in_dir;

                for(uint32_t ii = 0; ii < filesToCreate; ++ii)
                {
                    auto path = dir + r_fs::PATH_SLASH + r_uuid::generate();

                    r_append_file::allocate(path,
                                            cfg.file_allocation_bytes,
                                            cfg.max_indexes_per_file,
                                            "00000000-0000-0000-0000-000000000000");

                    paths.push_back(path);
                }
            }
        }
    }

    r_file_index::allocate(cfg.index_path);

    r_file_index db(cfg.index_path);

    random_shuffle(begin(paths), end(paths));

    r_sqlite_conn conn(cfg.index_path, true);

    if(!paths.empty())
    {
        conn.exec("DELETE FROM segment_files;");
        conn.exec("DELETE FROM sqlite_sequence;");
    }

    conn.exec("BEGIN");
    size_t flushCounter = 0;
    for(auto path : paths)
    {
        db.create_invalid_segment_file(conn, path, "00000000-0000-0000-0000-000000000000");
        ++flushCounter;

        if((flushCounter % 65535) == 0)
        {
            conn.exec("COMMIT");
            conn.exec("BEGIN");
        }
    }
    conn.exec("COMMIT");
}


// {
//     "storage_config": {
//         "file_allocation_bytes": "10485760",           #optional
//         "max_files_in_dir": "10000",                   #optional
//         "max_indexes_per_file": "3200",                #optional
//         "index_path": "/data/vss/index",
//         "data_sources_path": "/data/vss/data_sources",
//         "file_systems": [
//             {
//                 "name": "fs1",
//                 "path": "/mnt/fs1",
//                 "reserve_bytes": "10485760"
//             },
//             {
//                 "name": "fs2",
//                 "path": "/mnt/fs2",
//                 "reserve_bytes": "10485760"
//             }
//         ]
//     }
// }

void r_storage_engine::configure_storage(const string& config)
{
    auto docj = json::parse(config);

    if(docj.find("storage_config") == docj.end())
        R_STHROW(r_invalid_argument_exception, ("storage_config object not found."));

    auto cfgj = docj["storage_config"];

    r_engine_config cfg;

    cfg.file_allocation_bytes = (1024*1024*10);
    if(cfgj.find("file_allocation_bytes") != cfgj.end())
        cfg.file_allocation_bytes = r_string_utils::s_to_uint64(cfgj["file_allocation_bytes"].get<string>());

    cfg.max_files_in_dir = 10000;
    if(cfgj.find("max_files_in_dir") != cfgj.end())
        cfg.max_files_in_dir = r_string_utils::s_to_uint32(cfgj["max_files_in_dir"].get<string>());

    cfg.max_indexes_per_file = 3200;
    if(cfgj.find("max_indexes_per_file") != cfgj.end())
        cfg.max_indexes_per_file = r_string_utils::s_to_uint32(cfgj["max_indexes_per_file"].get<string>());

    cfg.index_path = cfgj["index_path"].get<string>();

    if(cfgj.find("file_systems") == cfgj.end())
        R_STHROW(r_invalid_argument_exception, ("config is missing file_systems array."));

    vector<r_file_system> fileSystems;
    cfg.file_systems.reserve(cfgj["file_systems"].size());

    for(auto& fs : cfgj["file_systems"])
        cfg.file_systems.emplace_back(_parse_file_system(fs.dump()));

    _fill_file_systems(cfg);   
}
