
#ifndef r_disco_r_onvif_provider_h
#define r_disco_r_onvif_provider_h

#include "r_disco/r_provider.h"
#include "r_disco/r_stream_config.h"
#include "r_utils/r_nullable.h"
#include "r_onvif/r_onvif_session.h"
#include <chrono>

namespace r_disco
{

class r_agent;

class r_onvif_provider : public r_provider
{
public:
    r_onvif_provider(const std::string& top_dir, r_agent* agent);
    ~r_onvif_provider();

    virtual std::vector<r_stream_config> poll();

private:
    std::vector<r_stream_config> _fetch_configs(const std::string& top_dir);
    std::string _top_dir;
    r_onvif::r_onvif_session _session;
    r_agent* _agent;
};

}

#endif
