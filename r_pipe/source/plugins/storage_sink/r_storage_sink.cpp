
#include "r_pipe/plugins/storage_sink/r_storage_sink.h"

using namespace r_av;
using namespace r_storage;
using namespace std;

r_storage_sink::r_storage_sink(const string& indexPath, const string& dataSourceID, const string& type) :
    _indexPath(indexPath),
    _dataSourceID(dataSourceID),
    _type(type),
    _params(),
    _index(_indexPath),
    _sf(),
    _file(),
    _st(),
    _frames(),
    _gopSize(0)
{
    _frames.reserve(30);
}

r_av::r_packet r_storage_sink::process(r_av::r_packet& pkt)
{
    if(!_st)
        _st = make_shared<r_stream_time>(pkt.get_time_base().second);

    // We queue up a gop in _frames. We write when we get the next gops key (but
    // we dont write that key, we queue it in _frames after we write all the
    // _frames).
    if(pkt.is_key() && !_frames.empty())
    {
        for(auto& f : _frames)
        {
            auto ts = _st->current(f.get_pts());

            if(!_file)
            {
                _sf = _index.recycle_append(ts, _dataSourceID, _type, f.get_sdp(), [this, &ts, &f](const segment_file& sf){
                    printf("RECORDING TO FILE A: %s\n", sf.path.c_str());
                    fflush(stdout);
                    r_append_file::reset(sf.path, this->_dataSourceID);
                    this->_file = make_shared<r_append_file>(sf.path);
                    this->_file->append(ts, (f.is_key())?r_storage::FLAG_KEY:0, f.map(), f.get_data_size());
                });
            }
            else
            {
                if(!_file->fits(_gopSize, _frames.size()))
                {
                    _index.update_end_time(_sf, _file->last_key());
                    _sf = _index.recycle_append(ts, _dataSourceID, _type, f.get_sdp(), [this, &ts, &f](const segment_file& sf){
                        printf("RECORDING TO FILE B: %s\n", sf.path.c_str());
                        fflush(stdout);
                        r_append_file::reset(sf.path, this->_dataSourceID);        
                        this->_file = make_shared<r_append_file>(sf.path);
                        this->_file->append(ts, (f.is_key())?r_storage::FLAG_KEY:0, f.map(), f.get_data_size());
                    });
                }
                else
                {
                    this->_file->append(ts, (f.is_key())?r_storage::FLAG_KEY:0, f.map(), f.get_data_size());
                }
            }
        }

        _frames.clear();
        _gopSize = pkt.get_data_size();
        _frames.push_back(pkt);
    }
    else
    {
        _gopSize += pkt.get_data_size();
        _frames.push_back(pkt);
    }

    return pkt;
}

void r_storage_sink::commit_params()
{
}
