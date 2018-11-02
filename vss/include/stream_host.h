
#ifndef _vss_stream_host_h
#define _vss_stream_host_h

#include "data_sources.h"
#include "r_pipe/r_control.h"
#include <map>
#include <string>

struct stream_host
{
    std::map<std::string, std::unique_ptr<r_pipe::r_control>> controls;
};

#endif
