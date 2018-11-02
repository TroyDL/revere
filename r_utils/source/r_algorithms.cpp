
#include "r_utils/r_algorithms.h"
#include "r_utils/r_exception.h"

// returns pointer to first element between start and end which does not compare less than target
uint8_t* r_utils::lower_bound_bytes( uint8_t* start,
                                     uint8_t* end,
                                     uint8_t* target,
                                     size_t elementSize,
                                     std::function<int(const uint8_t* p1, const uint8_t* target)> cmp )
{
    if( (start >= end) || (((size_t)(end-start)) < elementSize) )
        R_STHROW(r_invalid_argument_exception, ("Empty array!"));

    uint64_t len = (end-start)/elementSize;
    uint64_t half;
    uint8_t* middle;

    while( len > 0 )
    {
        half = len >> 1;
        middle = start;
        middle += half * elementSize;
        if( cmp( middle, target ) == -1 )
        {
            start = middle;
            start += elementSize;
            len = len - half - 1;
        }
        else len = half;
    }

    return start;
}
