
#ifndef r_disco_r_manual_provider_h
#define r_disco_r_manual_provider_h

#include "r_disco/r_provider.h"
#include "r_disco/r_stream_config.h"

namespace r_disco
{

class r_manual_provider : public r_provider
{
public:
    r_manual_provider(const std::string& top_dir);
    ~r_manual_provider();

    virtual std::vector<r_stream_config> poll();

private:
    std::vector<r_stream_config> _configs;
};

}

#endif
