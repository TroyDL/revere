
#ifndef _r_storage_sink_h
#define _r_storage_sink_h

#include "r_pipe/r_filter.h"
#include "r_storage/r_file_index.h"
#include "r_storage/r_segment_file.h"
#include "r_storage/r_append_file.h"
#include "r_storage/r_stream_time.h"
#include "r_utils/r_nullable.h"
#include <string>
#include <map>
#include <memory>

class r_storage_sink : public r_pipe::r_filter
{
public:
    r_storage_sink(const std::string& indexPath, const std::string& dataSourceID, const std::string& type);
    virtual ~r_storage_sink() noexcept {}

    virtual r_av::r_packet process(r_av::r_packet& pkt);

    virtual void set_param(const std::string& name, const std::string& val) { _params[name] = val; }
    virtual void commit_params();

private:
    std::string _indexPath;
    std::string _dataSourceID;
    std::string _type;
    std::map<std::string, std::string> _params;
    r_storage::r_file_index _index;
    r_storage::segment_file _sf;
    std::shared_ptr<r_storage::r_append_file> _file;
    std::shared_ptr<r_storage::r_stream_time> _st;
    std::vector<r_av::r_packet> _frames;
    uint64_t _gopSize;
};

#endif
