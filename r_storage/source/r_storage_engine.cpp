
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
    rfs.reserve_bytes = fsj["reserve_bytes"].get<uint64_t>();

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

void r_storage_engine::configure_storage(const string& config)
{
    auto docj = json::parse(config);

    if(docj.find("storage_config") == docj.end())
        R_STHROW(r_invalid_argument_exception, ("storage_config object not found."));

    auto cfgj = docj["storage_config"];

    r_engine_config cfg;

    cfg.file_allocation_bytes = (1024*1024*10);
    if(cfgj.find("file_allocation_bytes") != cfgj.end())
        cfg.file_allocation_bytes = cfgj["file_allocation_bytes"].get<uint64_t>();

    cfg.max_files_in_dir = 10000;
    if(cfgj.find("max_files_in_dir") != cfgj.end())
        cfg.max_files_in_dir = cfgj["max_files_in_dir"].get<uint32_t>();

    cfg.max_indexes_per_file = 3200;
    if(cfgj.find("max_indexes_per_file") != cfgj.end())
        cfg.max_indexes_per_file = cfgj["max_indexes_per_file"].get<uint32_t>();

    cfg.index_path = cfgj["index_path"].get<string>();

    if(cfgj.find("file_systems") == cfgj.end())
        R_STHROW(r_invalid_argument_exception, ("config is missing file_systems array."));

    vector<r_file_system> fileSystems;
    cfg.file_systems.reserve(cfgj["file_systems"].size());

    for(auto& fs : cfgj["file_systems"])
        cfg.file_systems.emplace_back(_parse_file_system(fs.dump()));

    _fill_file_systems(cfg);   
}

string _read_line()
{
    char lineBuffer[1024];
    memset(&lineBuffer[0], 0, 1024);
    fgets(&lineBuffer[0], 1024, stdin);
    return r_string_utils::strip_eol(string(lineBuffer));
}

int _to_int_or_default(const string& line, int def)
{
    if(line.length() > 0)
        return r_string_utils::s_to_int(line);
    return def;
}

string _to_string_or_default(const string& line, const string& def)
{
    if(line.length() > 0)
        return line;
    return def;
}

void r_storage_engine::create_config(const std::string& configPath)
{
    printf("Revere Interactive Storage Configuration\n\n");

    printf("You will be asked a series of questions and from this we will create a configuration for your video storage.\n");
    printf("In most cases, the defaults will probably work.\n\n");

    printf("Configuring directory: %s\n",configPath.c_str());
    if(!r_fs::file_exists(configPath))
        r_fs::mkdir(configPath);

    printf("How big should each video file be in megabytes? (Default 10mb): ");
    int fileSizeInMB = _to_int_or_default(_read_line(), 10);

    printf("What is the most number of files that should be put in one directory? (Default 10000): ");
    int maxFilesInDir = _to_int_or_default(_read_line(), 10000);

    printf("How many indexes per file do you need? (Default 3200): ");
    int indexesPerFile = _to_int_or_default(_read_line(), 3200);
    
    printf("Where should the master index be located? (Default /data/vss/index): ");
    string indexPath = _to_string_or_default(_read_line(), "/data/vss/index");

    printf("Where should we put the camera storage database? (Default /data/vss/data_sources): ");
    string dataSourcesPath = _to_string_or_default(_read_line(), "/data/vss/data_sources");

    printf("How many file systems would you like to configure? (Default 1): ");
    int numFileSystems = _to_int_or_default(_read_line(), 1);

    vector<r_file_system> fileSystems;    
    
    for(int i = 0; i < numFileSystems; ++i)
    {
        string defaultFileSystemName = r_string_utils::format("fs%d",i);

        printf("Please enter a name for this filesystem (Default \"%s\"): ", defaultFileSystemName.c_str());
        string fileSystemName = _to_string_or_default(_read_line(), defaultFileSystemName);

        if(!r_fs::file_exists(configPath + r_fs::PATH_SLASH + "video"))
            r_fs::mkdir(configPath + r_fs::PATH_SLASH + "video");

        string defaultFileSystemPath = configPath + r_fs::PATH_SLASH + "video" + r_fs::PATH_SLASH + fileSystemName;
        printf("Please enter the path to this filesystem (%s) (Default \"%s\"): ", fileSystemName.c_str(), defaultFileSystemPath.c_str());
        string fileSystemPath = _to_string_or_default(_read_line(), defaultFileSystemPath);

        if(!r_fs::file_exists(fileSystemPath))
            r_fs::mkdir(fileSystemPath);

        printf("Please enter the free space reserve for this filesystem (%s) in megabytes (Default 10mb): ", fileSystemName.c_str());
        int freeSpaceReserveMB = _to_int_or_default(_read_line(), 10);

        r_file_system fs;
        fs.name = fileSystemName;
        fs.path = fileSystemPath;
        fs.reserve_bytes = (1024*1024) * freeSpaceReserveMB;
        fileSystems.push_back(fs);
    }

    string fs;
    for(auto i = fileSystems.begin(), e = fileSystems.end(); i != e; ++i)
    {
        fs += r_string_utils::format("             {\n"
                                     "                 \"name\": \"%s\",\n"
                                     "                 \"path\": \"%s\",\n"
                                     "                 \"reserve_bytes\": %llu\n"
                                     "             }%s\n",
                                     i->name.c_str(),
                                     i->path.c_str(),
                                     i->reserve_bytes,
                                     (next(i) != e)?",":"");
    }

    string cfg = r_string_utils::format("{\n"
                                        "    \"storage_config\": {\n"
                                        "         \"file_allocation_bytes\": %d,\n"
                                        "         \"max_files_in_dir\": %d,\n"
                                        "         \"max_indexes_per_file\": %d,\n"
                                        "         \"index_path\": \"%s\",\n"
                                        "         \"data_sources_path\": \"%s\",\n"
                                        "         \"file_systems\": [\n"
                                        "%s"
                                        "         ]\n"
                                        "      }\n"
                                        "}\n\n",
                                        (1024*1024) * fileSizeInMB,
                                        maxFilesInDir,
                                        indexesPerFile,
                                        indexPath.c_str(),
                                        dataSourcesPath.c_str(),
                                        fs.c_str());

    r_fs::write_file((uint8_t*)cfg.c_str(), cfg.length(), configPath + r_fs::PATH_SLASH + "config.json");

    printf("Done.\nNow re-run vss like this: vss OR vss %s\n", configPath.c_str());
}
