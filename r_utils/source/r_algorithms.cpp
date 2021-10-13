
#include "r_utils/r_algorithms.h"
#include "r_utils/r_exception.h"

// returns pointer to first element between start and end which does not compare less than target
uint8_t* r_utils::lower_bound_bytes( uint8_t* start,
                                     uint8_t* end,
                                     uint8_t* target,
                                     size_t elementSize,
                                     std::function<int(const uint8_t* p1, const uint8_t* target)> cmp )
{
    const size_t N = (end - start) / elementSize;
    size_t mid;
    size_t low = 0;
    size_t high = N;

    while(low < high) {
        mid = low + (high - low) / 2;
        auto res = cmp(target, start + (mid * elementSize));
        if(res <= 0)
            high = mid;
        else
            low = mid + 1;
    }

    auto res = cmp(start + (low * elementSize), target);
    if(low < N && res < 0)
        low++;

    return start + (low * elementSize);
}
