#include "r_disco/r_device_info.h"

bool r_disco::operator < (const r_device_info& a, const r_device_info& b)
{
    return a.unique_id < b.unique_id;
}
