
#ifndef r_discovery_r_query_device_info_h
#define r_discovery_r_query_device_info_h

#include "r_disco/r_device_info.h"
#include "r_utils/r_nullable.h"
#include <string>

namespace r_disco
{

class r_query_device_info
{
public:
    virtual r_utils::r_nullable<r_device_info> get_device_info(const std::string& ssdpNotifyMessage) = 0;
};

}

#endif
