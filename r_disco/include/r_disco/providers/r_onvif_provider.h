
#ifndef r_disco_r_onvif_provider_h
#define r_disco_r_onvif_provider_h

#include "r_disco/r_stream_config.h"
#include "r_utils/r_nullable.h"
#include "r_onvif/r_onvif_session.h"
#include <chrono>
#include <map>

namespace r_disco
{

class r_agent;

class r_onvif_provider
{
public:
    r_onvif_provider(const std::string& top_dir, r_agent* agent);
    ~r_onvif_provider();

    std::vector<r_stream_config> poll();

    r_utils::r_nullable<r_stream_config> interrogate_camera(
        const std::string& id,
        const std::string& camera_name,
        const std::string& ipv4,
        const std::string& xaddrs,
        const std::string& address,
        r_utils::r_nullable<std::string> username,
        r_utils::r_nullable<std::string> password
    );

private:
    std::vector<r_stream_config> _fetch_configs(const std::string& top_dir);
    void _cache_check_expiration(const std::string& id);
    std::string _top_dir;
    r_onvif::r_onvif_session _session;
    r_agent* _agent;

    struct _r_onvif_provider_cache_entry
    {
        std::chrono::steady_clock::time_point created;
        r_stream_config config;
    };

    std::map<std::string, _r_onvif_provider_cache_entry> _cache;
};

}

#endif
