
#ifndef __r_pipe_r_filter_group_h
#define __r_pipe_r_filter_group_h

#include "r_pipe/r_filter.h"
#include "r_av/r_packet.h"
#include <list>
#include <memory>

namespace r_pipe
{

class r_filter_group : public r_filter
{
public:
    r_filter_group() = default;
    virtual ~r_filter_group() throw() {}

    void add(std::shared_ptr<r_filter> filter) { _filters.push_back(filter); }

    virtual r_av::r_packet process(r_av::r_packet& pkt)
    {
        for(std::shared_ptr<r_filter> f : _filters)
            pkt = f->process(pkt);

        return pkt;
    }

    virtual void set_param( const std::string&, const std::string& )
    {
        R_THROW(("Cannot set parameters on a filter_group as a whole."));
    }

    virtual void commit_params()
    {
        R_THROW(("Cannot commit parameters on a filter_group as a whole."));
    }

private:
    std::list<std::shared_ptr<r_filter> > _filters;
};

}

#endif