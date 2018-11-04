#ifndef __r_pipe_r_filter_h
#define __r_pipe_r_filter_h

#include "r_pipe/r_stateful.h"
#include "r_utils/r_string_utils.h"
#include "r_av/r_packet.h"
#include <memory>

namespace r_pipe
{

class r_filter : public r_stateful
{
public:
    r_filter() = default;
    virtual ~r_filter() noexcept {}

    virtual r_av::r_packet process(r_av::r_packet& pkt) = 0;

    virtual void set_param(const std::string& name, const std::string& val) = 0;
    virtual void commit_params() = 0;

    virtual void run() {}
    virtual void stop() {}
};

}

#endif
