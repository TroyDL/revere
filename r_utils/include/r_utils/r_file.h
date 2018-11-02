

#ifndef r_utils_r_file_h
#define r_utils_r_file_h

#define _LARGE_FILE_API
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "r_utils/r_string.h"
#include "r_utils/r_exception.h"
#include <stdlib.h>
#include <memory>
#include <vector>
#include <future>

class test_r_utils_r_file;

namespace r_utils
{

class r_file final
{
    friend class ::test_r_utils_r_file;
public:
    r_file() : _f(nullptr) {}
    r_file(const r_file&) = delete;
	r_file(r_file&& obj) noexcept;
	~r_file() noexcept;

    r_file& operator = (r_file&) = delete;
	r_file& operator = (r_file&& obj) noexcept;

	operator FILE*() const { return _f; }

	static r_file open(const std::string& path, const std::string& mode)
    {
        r_file obj;
        obj._f = fopen(path.c_str(), mode.c_str());
        if(!obj._f)
            R_STHROW(r_not_found_exception, ("Unable to open: %s",path.c_str()));
        return std::move(obj);
    }

	void close() { fclose(_f); _f = nullptr; }

private:
    FILE* _f;
};

class r_path final
{
    struct path_parts
    {
        std::string path;
        std::string glob;
    };

 public:
    r_path(const std::string& glob);
    r_path(const r_path&) = delete;
    r_path(r_path&& obj) noexcept;

    ~r_path() noexcept;

    r_path& operator=(const r_path&) = delete;
    r_path& operator=(r_path&& obj) noexcept;

    void open_dir(const std::string& glob);
    bool read_dir(std::string& fileName);

 private:
    void _clear() noexcept;
    path_parts _get_path_and_glob(const std::string& glob) const;

    path_parts _pathParts;
    bool _done;
    DIR* _d;
};

namespace r_fs
{

extern const std::string PATH_SLASH;

enum r_file_type
{
    R_REGULAR,
    R_DIRECTORY
};

struct r_file_info
{
    std::string file_name;
    uint64_t file_size;
    r_file_type file_type;
    uint32_t optimal_block_size;
    std::chrono::system_clock::time_point access_time;
    std::chrono::system_clock::time_point modification_time;
};

int stat(const std::string& fileName, struct r_file_info* fileInfo);
std::vector<uint8_t> read_file(const std::string& path);
void write_file(const uint8_t* bytes, size_t len, const std::string& path);
void atomic_rename_file(const std::string& oldPath, const std::string& newPath);
bool file_exists(const std::string& path);
bool is_reg(const std::string& path);
bool is_dir(const std::string& path);
int fallocate(FILE* file, uint64_t size);
void break_path(const std::string& path, std::string& dir, std::string& fileName);
std::string temp_file_name(const std::string& dir, const std::string& baseName = std::string());
void get_fs_usage(const std::string& path, uint64_t& size, uint64_t& free);
uint64_t file_size(const std::string& path);
void mkdir(const std::string& path);

}

}

#endif
