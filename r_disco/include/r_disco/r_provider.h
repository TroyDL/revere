
#ifndef r_disco_r_provider_h
#define r_disco_r_provider_h

#include <vector>
#include "r_disco/r_stream_config.h"

namespace r_disco
{

class r_provider
{
public:
    virtual std::vector<r_stream_config> poll() = 0;
};

}

#endif
