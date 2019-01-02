
#ifndef r_discovery_r_argus_device_info_agent_h
#define r_discovery_r_argus_device_info_agent_h

#include "r_disco/interfaces/r_query_device_info.h"
#include <string>

namespace r_disco
{

class r_argus_device_info_agent : public r_query_device_info
{
public:
    r_argus_device_info_agent();
    r_argus_device_info_agent(const r_argus_device_info_agent&) = delete;
    r_argus_device_info_agent(r_argus_device_info_agent&&) = delete;
    virtual ~r_argus_device_info_agent() noexcept;
    r_argus_device_info_agent& operator=(const r_argus_device_info_agent&) = delete;
    r_argus_device_info_agent& operator=(r_argus_device_info_agent&&) = delete;

    virtual r_utils::r_nullable<r_device_info> get_device_info(const std::string& ssdpNotifyMessage);
};

}

#endif
