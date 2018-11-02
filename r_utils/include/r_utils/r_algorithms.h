
#ifndef r_utils_r_lower_bound_h
#define r_utils_r_lower_bound_h

#include <cstdint>
#include <functional>

namespace r_utils
{

// returns pointer to first element between start and end which does not compare less than target
uint8_t* lower_bound_bytes(uint8_t* start,
    				       uint8_t* end,
				           uint8_t* target,
				           size_t elementSize,
				           std::function<int(const uint8_t* p1, const uint8_t* target)> cmp);

}

#endif