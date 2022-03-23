
#include "utils.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_file.h"

#ifdef IS_WINDOWS
#include <shlobj_core.h>
#endif

#ifdef IS_LINUX
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

#include <deque>
#include <numeric>
#include <iterator>

using namespace std;
using namespace r_utils;

string top_dir()
{
#ifdef IS_WINDOWS
    wchar_t szPath[MAX_PATH];

    auto ret = SHGetFolderPathW(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, szPath);

    if(ret != S_OK)
        R_THROW(("Could not get path to documents folder."));

    return r_string_utils::convert_wide_string_to_multi_byte_string(szPath)  + r_fs::PATH_SLASH + "revere" + r_fs::PATH_SLASH + "revere";
#endif

#ifdef IS_LINUX
    auto res = sysconf(_SC_GETPW_R_SIZE_MAX);
    if(res == -1)
        R_THROW(("Could not get path to documents folder."));
    vector<char> buffer(res);
    passwd pwd;
    passwd* result;
    auto ret = getpwuid_r(getuid(), &pwd, buffer.data(), buffer.size(), &result);
    if(ret != 0)
        R_THROW(("Could not get path to documents folder."));
    return string(pwd.pw_dir) + r_fs::PATH_SLASH + "Documents" + r_fs::PATH_SLASH + "revere" + r_fs::PATH_SLASH + "revere";
#endif
}

string sub_dir(const string& subdir)
{
    auto top = top_dir();
    auto sd = top + r_fs::PATH_SLASH + subdir;

    if(!r_fs::file_exists(sd))
        r_fs::mkdir(sd);

    return sd;
}

std::string join_path(const std::string& path, const std::string& fileName)
{
    return path + r_fs::PATH_SLASH + fileName;
}

vector<pair<int64_t, int64_t>> find_contiguous_segments(const vector<int64_t>& times)
{
    // Since our input times are key frames and different cameras may have different GOP sizes we need to discover for
    // each camera what the normal space between key frames is. We then multiply this value by 1.5 to find our segment
    // threshold.

    int64_t threshold = 0;
    if(times.size() > 1)
    {
        vector<int64_t> deltas;
        adjacent_difference(begin(times), end(times), back_inserter(deltas));
        threshold = (deltas.size() > 2)?(int64_t)((accumulate(begin(deltas)+1, end(deltas), .0) / deltas.size()) * 1.5):0;
    }

    deque<int64_t> timesd;
    for (auto t : times) {
        timesd.push_back(t);
    }

    vector<vector<int64_t>> segments;
    vector<int64_t> current;

    // Walk the times front to back appending new times to current as long as they are within the threshold.
    // If a new time is outside the threshold then we have a new segment so push current onto segments and
    // start a new current.

    while(!timesd.empty())
    {
        auto t = timesd.front();
        timesd.pop_front();

        if(current.empty())
            current.push_back(t);
        else
        {
            auto delta = t - current.back();
            if(delta > threshold && !current.empty())
            {
                segments.push_back(current);
                current = {t};
            }
            else current.push_back(t);
        }
    }

    // since we push current when we find a gap bigger than threshold we need to push the last current segment.
    if(!current.empty())
        segments.push_back(current);

    vector<pair<int64_t, int64_t>> output;
    for(auto& s : segments)
        output.push_back(make_pair(s.front(), s.back()));

    return output;
}
