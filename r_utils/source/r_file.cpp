
#include "r_utils/r_file.h"
#include "r_utils/r_string.h"
#include <random>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <fnmatch.h>
#include <sys/types.h>

using namespace r_utils;
using namespace std;

const string r_utils::r_fs::PATH_SLASH = "/";

r_file::r_file(r_file&& obj) noexcept :
    _f(std::move(obj._f))
{
    obj._f = nullptr;
}

r_file::~r_file() noexcept
{
    if(_f)
        fclose(_f);
}

r_file& r_file::operator = (r_file&& obj) noexcept
{
    if(_f)
        fclose(_f);

    _f = std::move(obj._f);
    obj._f = nullptr;
    return *this;
}

r_path::r_path(const string& glob) :
    _pathParts(),
    _done(false),
    _d(nullptr)
{
    open_dir(glob);
}

r_path::r_path(r_path&& obj) noexcept :
    _pathParts(std::move(obj._pathParts)),
    _done(std::move(obj._done)),
    _d(std::move(obj._d))
{
    obj._d = nullptr;
    obj._pathParts = path_parts();
}

r_path::~r_path() noexcept
{
    _clear();
}

r_path& r_path::operator=(r_path&& obj) noexcept
{
    _clear();

    _pathParts = std::move(obj._pathParts);
    obj._pathParts = path_parts();
    _done = std::move(obj._done);
    _d = std::move(obj._d);
    obj._d = nullptr;

    return *this;
}

void r_path::open_dir(const string& glob)
{
    _clear();

    auto parts = _get_path_and_glob(glob);

    _d = opendir(parts.path.c_str());
    if(!_d)
        R_STHROW(r_not_found_exception, ("Unable to open directory: %s",parts.path.c_str()));
    _done = false;
    _pathParts = parts;
}

bool r_path::read_dir(string& fileName)
{
    if(_done)
        return false;

    struct dirent* entry = nullptr;
    while((entry = readdir(_d)))
    {
        string entryName = entry->d_name;
        if((entryName != ".") && (entryName != ".."))
        {
            if(!_pathParts.glob.empty())
            {
                if(fnmatch(_pathParts.glob.c_str(), entryName.c_str(), 0) == 0)
                {
                    fileName = entryName;
                    return true;
                }
            }
            else
            {
                fileName = entryName;
                return true;
            }
        }
    }

    if(entry == nullptr)
        _done = true;

    return false;
}

void r_path::_clear() noexcept
{
    if(_d)
    {
        closedir(_d);
        _d = nullptr;
    }
}

r_path::path_parts r_path::_get_path_and_glob(const string& glob) const
{
    if(r_fs::is_reg(glob))
        R_STHROW(r_internal_exception, ("Glob passed to r_path specifies a regular file."));

    path_parts parts;

    if(r_fs::is_dir(glob))
    {
        parts.path = glob;
        parts.glob = "";
    }
    else
    {
        const size_t lastSlash = glob.rfind(r_fs::PATH_SLASH);
        parts.path = glob.substr( 0, lastSlash);
        parts.glob = (glob.size() > (lastSlash + 1)) ? glob.substr(lastSlash + 1, glob.size() - (lastSlash + 1)) : "";
    }

    return parts;

}

int r_utils::r_fs::stat(const string& file_name, struct r_file_info* file_info)
{
    struct stat sfi;

    if(stat(file_name.c_str(), &sfi) == 0)
    {
        file_info->file_name = file_name;
        file_info->file_size = sfi.st_size;
        file_info->file_type = (sfi.st_mode & S_IFDIR) ? R_DIRECTORY : R_REGULAR;
        file_info->optimal_block_size = sfi.st_blksize;

        file_info->access_time = std::chrono::system_clock::from_time_t(sfi.st_atime);
        file_info->modification_time = std::chrono::system_clock::from_time_t(sfi.st_mtime);

        return 0;
    }

    return -1;
}

vector<uint8_t> r_utils::r_fs::read_file(const string& path)
{
    struct r_file_info fi;
    r_utils::r_fs::stat(path, &fi);

    uint32_t numBlocks = (fi.file_size > fi.optimal_block_size) ? fi.file_size / fi.optimal_block_size : 0;
    uint32_t remainder = (fi.file_size > fi.optimal_block_size) ? fi.file_size % fi.optimal_block_size : fi.file_size;

    vector<uint8_t> out(fi.file_size);
    uint8_t* writer = &out[0];

    auto f = r_file::open(path, "rb");

    while(numBlocks > 0)
    {
        auto blocksRead = fread(writer, fi.optimal_block_size, numBlocks, f);
        writer += blocksRead * fi.optimal_block_size;
        numBlocks -= blocksRead;
    }

    if(remainder > 0)
        fread(writer, 1, remainder, f);

    return out;
}

void r_utils::r_fs::write_file(const uint8_t* bytes, size_t len, const string& path)
{
    auto f = r_file::open(path, "w+b");
    struct r_file_info fi;
    r_utils::r_fs::stat(path, &fi);

    uint32_t numBlocks = (len > fi.optimal_block_size) ? len / fi.optimal_block_size : 0;
    uint32_t remainder = (len > fi.optimal_block_size) ? len % fi.optimal_block_size : len;

    while(numBlocks > 0)
    {
        auto blocksWritten = fwrite(bytes, fi.optimal_block_size, numBlocks, f);
        bytes += blocksWritten * fi.optimal_block_size;
        numBlocks -= blocksWritten;
    }

    if(remainder > 0)
        fwrite(bytes, 1, remainder, f);
}

void r_utils::r_fs::atomic_rename_file(const string& oldPath, const string& newPath)
{
    if(rename(oldPath.c_str(), newPath.c_str()) < 0)
        R_STHROW(r_internal_exception, ("Unable to rename %s to %s", oldPath.c_str(), newPath.c_str()));
}

bool r_utils::r_fs::file_exists(const string& path)
{
    struct r_utils::r_fs::r_file_info rfi;
    return (r_utils::r_fs::stat(path, &rfi) == 0) ? true : false;
}

bool r_utils::r_fs::is_reg(const string& path)
{
    struct r_utils::r_fs::r_file_info rfi;
    if(r_utils::r_fs::stat(path, &rfi) == 0)
    {
        if(rfi.file_type == r_utils::r_fs::R_REGULAR)
            return true;
    }

    return false;
}

bool r_utils::r_fs::is_dir(const string& path)
{
    struct r_utils::r_fs::r_file_info rfi;
    if(r_utils::r_fs::stat(path, &rfi) == 0)
    {
        if(rfi.file_type == r_utils::r_fs::R_DIRECTORY)
            return true;
    }

    return false;
}

int r_utils::r_fs::fallocate(FILE* file, uint64_t size)
{
    return posix_fallocate64(fileno(file), 0, size);
}

void r_utils::r_fs::break_path(const string& path, string& dir, string& fileName)
{
    auto p = path;
    while(r_utils::r_string_utils::ends_with(p, PATH_SLASH))
        p = p.substr(0, p.length() - 1);

    size_t rslash = p.rfind(PATH_SLASH);
    if(rslash == string::npos)
    {
        dir = "";
        fileName = p;
    }
    else
    {
        dir = p.substr(0, p.rfind(PATH_SLASH));
        fileName = p.substr(p.rfind(PATH_SLASH) + 1);
    }
}

string r_utils::r_fs::temp_file_name(const string& dir, const string& baseName)
{
    random_device rd;
    mt19937 mersenne_twister_rng(rd());
    uniform_int_distribution<int> uid(65, 90);

    string foundPath;

    while(true)
    {
        foundPath = (!dir.empty())? dir + PATH_SLASH : string(".") + PATH_SLASH;

        if(!baseName.empty())
            foundPath += baseName;

        for(int i = 0; i < 8; ++i)
            foundPath += (char)uid(mersenne_twister_rng);

        if(!r_utils::r_fs::file_exists(foundPath))
            return foundPath;
    }
}

void r_utils::r_fs::get_fs_usage(const string& path, uint64_t& size, uint64_t& free)
{
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) < 0)
        R_STHROW(r_not_found_exception, ("Unable to statvfs() path."));

    size = (uint64_t)stat.f_blocks * (uint64_t)stat.f_frsize;
    free = (uint64_t)stat.f_bavail * (uint64_t)stat.f_bsize;
}

uint64_t r_utils::r_fs::file_size(const std::string& path)
{
    uint64_t fileSize = 0;

    if(r_fs::is_dir(path))
    {
        r_path files(path);

        string fileName;
        while(files.read_dir(fileName))
            fileSize += file_size(path + r_fs::PATH_SLASH + fileName );
    }
    else
    {
        r_file_info fileInfo;
        stat(path, &fileInfo);

        fileSize += fileInfo.file_size;
    }

    return fileSize;
}

void r_utils::r_fs::mkdir(const std::string& path)
{
    if(::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
        R_STHROW(r_internal_exception, ("Unable to make directory: %s",path.c_str()));
}
