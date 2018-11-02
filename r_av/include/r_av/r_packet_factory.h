#ifndef __r_av_r_packet_factory_h
#define __r_av_r_packet_factory_h

#include "r_utils/r_exception.h"
#include "r_av/r_packet.h"
#include <memory>

namespace r_av
{

class r_packet_factory
{
public:
    virtual r_packet get(size_t sz) = 0;
};

class r_packet_factory_default : public r_packet_factory
{
public:
    virtual r_packet get(size_t sz) { return r_packet(sz); }
};

}

#endif
