#ifndef __r_pipe_r_source_h
#define __r_pipe_r_source_h

#include "r_pipe/r_stateful.h"
#include "r_av/r_packet.h"
#include "r_av/r_demuxer.h"
#include <memory>
#include <functional>

namespace r_pipe
{

class r_source : public r_stateful
{
public:
    r_source() = default;
    virtual ~r_source() throw() {}

    virtual bool get( r_av::r_packet& pkt ) = 0;

    virtual void run() = 0;
    virtual void stop() = 0;

    virtual void set_param( const std::string& name, const std::string& val ) = 0;
    virtual void commit_params() = 0;

    void register_demuxer_init_cb( std::function<void(r_av::r_demuxer& demuxer)> cb ) { _demuxCB = cb; }

protected:
    std::function<void(r_av::r_demuxer& demuxer)> _demuxCB;
};

}

#endif
