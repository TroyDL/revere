
#ifndef __revere_utils_h
#define __revere_utils_h

#include <string>
#include <vector>
#include <map>
#include <cstdint>

std::string top_dir();

std::string sub_dir(const std::string& subdir);

std::vector<std::pair<int64_t, int64_t>> find_contiguous_segments(const std::vector<int64_t>& times);

#endif
