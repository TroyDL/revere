#include "r_av/r_demuxer.h"
#include "r_av/r_locky.h"
#include "r_av/r_utils.h"
#include "r_utils/r_file.h"
#include <utility>
#include <numeric>

extern "C"
{
#include "libavutil/opt.h"
}

using namespace r_av;
using namespace r_utils;
using namespace std;

static const size_t DEFAULT_EXTRADATA_BUFFER_SIZE = (1024*256);

r_demuxer::r_demuxer(const string& fileName, bool annexBFilter) :
    _fileName(fileName),
    _memoryIOContext(nullptr),
    _storage(),
    _pos(0),
    _context(nullptr),
    _eof(false),
    _deMuxPkt(),
    _filterPkt(),
    _streamTypes(),
    _videoStreamIndex(STREAM_TYPE_UNKNOWN),
    _audioPrimaryStreamIndex(STREAM_TYPE_UNKNOWN),
    _currentStreamIndex(STREAM_TYPE_UNKNOWN),
    _bsf(nullptr),
    _pf(std::make_shared<r_packet_factory_default>()),
    _outputTimeBases()
{
    if(!r_locky::is_registered())
        R_STHROW(r_internal_exception, ("Please call locky::register_ffmpeg() before using this class."));

    _deMuxPkt.size = 0;
    _deMuxPkt.data = nullptr;
    _filterPkt.size = 0;
    _filterPkt.data = nullptr;

    _open_streams();

    if(annexBFilter)
        _init_annexb_filter();
}

r_demuxer::r_demuxer(const uint8_t* buffer,
                      size_t bufferSize,
                      bool annexBFilter) :
    _fileName(),
    _memoryIOContext(nullptr),
    _storage(),
    _pos(0),
    _context(nullptr),
    _eof(false),
    _deMuxPkt(),
    _filterPkt(),
    _streamTypes(),
    _videoStreamIndex(STREAM_TYPE_UNKNOWN),
    _audioPrimaryStreamIndex(STREAM_TYPE_UNKNOWN),
    _currentStreamIndex(STREAM_TYPE_UNKNOWN),
    _bsf(nullptr),
    _pf(std::make_shared<r_packet_factory_default>()),
    _outputTimeBases()
{
    if(!r_locky::is_registered())
        R_STHROW(r_internal_exception, ("Please call locky::register_ffmpeg() before using this class."));

    _deMuxPkt.size = 0;
    _deMuxPkt.data = nullptr;
    _filterPkt.size = 0;
    _filterPkt.data = nullptr;

    _open_custom_io_context(buffer, bufferSize);

    _open_streams();

    if(annexBFilter)
        _init_annexb_filter();
}

r_demuxer::r_demuxer(r_demuxer&& obj) noexcept
{
    _fileName = std::move(obj._fileName);
    obj._fileName = string();
    _memoryIOContext = std::move(obj._memoryIOContext);
    obj._memoryIOContext = nullptr;
    _storage = std::move(obj._storage);
    obj._storage.clear();
    _pos = std::move(obj._pos);
    obj._pos = 0;
    _context = std::move(obj._context);
    obj._context = nullptr;
    _eof = std::move(obj._eof);
    obj._eof = false;
    _deMuxPkt = std::move(obj._deMuxPkt);
    obj._deMuxPkt.buf = nullptr;
    obj._deMuxPkt.data = nullptr;
    obj._deMuxPkt.size = 0;
    obj._deMuxPkt.side_data = nullptr;
    _filterPkt = std::move(obj._filterPkt);
    obj._filterPkt.buf = nullptr;
    obj._filterPkt.data = nullptr;
    obj._filterPkt.size = 0;
    obj._filterPkt.side_data = nullptr;
    _streamTypes = std::move(obj._streamTypes);
    _videoStreamIndex = std::move(obj._videoStreamIndex);
    _audioPrimaryStreamIndex = std::move(obj._audioPrimaryStreamIndex);
    _currentStreamIndex = std::move(obj._currentStreamIndex);
    _bsfc = std::move(obj._bsfc);
    _pf = std::move(obj._pf);
    _outputTimeBases = std::move(obj._outputTimeBases);
}

r_demuxer::~r_demuxer() noexcept
{
    _clear();
}

r_demuxer& r_demuxer::operator=(r_demuxer&& obj) noexcept
{
    _clear();

    _fileName = std::move(obj._fileName);
    obj._fileName = string();
    _memoryIOContext = std::move(obj._memoryIOContext);
    obj._memoryIOContext = nullptr;
    _storage = std::move(obj._storage);
    obj._storage.clear();
    _pos = std::move(obj._pos);
    obj._pos = 0;
    _context = std::move(obj._context);
    obj._context = nullptr;
    _eof = std::move(obj._eof);
    obj._eof = false;
    _deMuxPkt = std::move(obj._deMuxPkt);
    obj._deMuxPkt.buf = nullptr;
    obj._deMuxPkt.data = nullptr;
    obj._deMuxPkt.size = 0;
    obj._deMuxPkt.side_data = nullptr;
    _filterPkt = std::move(obj._filterPkt);
    obj._filterPkt.buf = nullptr;
    obj._filterPkt.data = nullptr;
    obj._filterPkt.size = 0;
    obj._filterPkt.side_data = nullptr;
    _streamTypes = std::move(obj._streamTypes);
    _videoStreamIndex = std::move(obj._videoStreamIndex);
    _audioPrimaryStreamIndex = std::move(obj._audioPrimaryStreamIndex);
    _currentStreamIndex = std::move(obj._currentStreamIndex);
    _bsfc = std::move(obj._bsfc);
    _pf = std::move(obj._pf);
    _outputTimeBases = std::move(obj._outputTimeBases);

    return *this;
}

string r_demuxer::get_sdp()
{
    char sdp_buffer[4096];
    av_sdp_create(&_context, 1, (char*)&sdp_buffer[0], 4096);
    return string(sdp_buffer);
}

string r_demuxer::get_file_name() const
{
    return _fileName;
}

vector<r_stream_info> r_demuxer::get_stream_types() const
{
    return _streamTypes;
}

int r_demuxer::get_video_stream_index() const
{
    if(_videoStreamIndex != STREAM_TYPE_UNKNOWN)
        return _videoStreamIndex;

    R_STHROW(r_internal_exception, ("Unable to locate video stream!"));
}

int r_demuxer::get_primary_audio_stream_index() const
{
    if(_audioPrimaryStreamIndex != STREAM_TYPE_UNKNOWN)
        return _audioPrimaryStreamIndex;

    R_STHROW(r_internal_exception, ("Unable to locate an audio stream!"));
}

pair<int,int> r_demuxer::get_time_base(int streamIndex) const
{
    return make_pair(_context->streams[streamIndex]->time_base.num, _context->streams[streamIndex]->time_base.den);
}

pair<int,int> r_demuxer::get_frame_rate(int streamIndex) const
{
    return make_pair(_context->streams[streamIndex]->r_frame_rate.num, _context->streams[streamIndex]->r_frame_rate.den);
}

void r_demuxer::set_output_time_base(int streamIndex, const pair<int, int>& tb)
{
    _outputTimeBases[streamIndex] = tb;
}

r_av_codec_id r_demuxer::get_stream_codec_id(int streamIndex) const
{
    return ffmpeg_codec_id_to_r_av_codec_id(_context->streams[streamIndex]->codecpar->codec_id);
}

r_pix_fmt r_demuxer::get_pix_format(int streamIndex) const
{
    return ffmpeg_pix_fmt_to_r_av_pix_fmt((AVPixelFormat)_context->streams[streamIndex]->codecpar->format);
}

pair<int,int> r_demuxer::get_resolution(int streamIndex) const
{
    return make_pair(_context->streams[streamIndex]->codecpar->width,
                     _context->streams[streamIndex]->codecpar->height);
}

vector<uint8_t> r_demuxer::get_extradata(int streamIndex) const
{
    auto s = _context->streams[streamIndex];

    vector<uint8_t> ed;

    if(s->codecpar->extradata_size > 0)
    {
        ed.resize(s->codecpar->extradata_size);
        memcpy(&ed[0], s->codecpar->extradata, s->codecpar->extradata_size);
    }

    return ed;
}

AVCodecParameters* r_demuxer::get_codec_parameters(int streamIndex) const
{
    return _context->streams[streamIndex]->codecpar;
}

bool r_demuxer::read_frame(int& streamIndex)
{
    _free_packet();

    if(av_read_frame(_context, &_deMuxPkt) >= 0)
    {
        streamIndex = _currentStreamIndex = _deMuxPkt.stream_index;

        if(streamIndex == _videoStreamIndex)
            _optional_annexb_filter();
    }
    else _eof = true;

    return !_eof;
}

bool r_demuxer::end_of_file() const
{
    return _eof;
}

bool r_demuxer::is_key() const
{
    if(!_eof)
    {
        const AVPacket* srcPkt = (_bsf) ? &_filterPkt : &_deMuxPkt;

        if((srcPkt->flags & AV_PKT_FLAG_KEY))
            return true;
    }

    return false;
}

r_packet r_demuxer::get() const
{
    r_packet pkt(r_filter_state_default);

    const AVPacket* srcPkt = (_bsf && _deMuxPkt.stream_index == _videoStreamIndex) ? &_filterPkt : &_deMuxPkt;

    if(_bsf && (srcPkt->stream_index == _videoStreamIndex))
    {
        pkt = _pf->get((size_t)srcPkt->size);
        pkt.set_data_size(srcPkt->size);
        memcpy(pkt.map(), srcPkt->data, srcPkt->size);
    }
    else
    {
        pkt = _pf->get((size_t)srcPkt->size);
        pkt.set_data_size(srcPkt->size);
        memcpy(pkt.map(), srcPkt->data, srcPkt->size);
    }

    pkt.set_pts(srcPkt->pts);
    pkt.set_dts(srcPkt->dts);
    pkt.set_duration(srcPkt->duration);
    pkt.set_time_base(get_time_base(srcPkt->stream_index));

    if(is_key())
        pkt.set_key(true);

    if(_outputTimeBases.find(_currentStreamIndex) != _outputTimeBases.end())
        pkt.rescale_time_base(_outputTimeBases.at(_currentStreamIndex));

    return pkt;
}

struct r_stream_statistics r_demuxer::get_video_stream_statistics(const string& fileName)
{
    struct r_stream_statistics result;

    vector<uint32_t> frameSizes;

    uint32_t indexFirstKey = 0;
    bool foundFirstKey = false;
    bool foundGOPSize = false;
    uint32_t currentIndex = 0;

    r_demuxer dm(fileName);

    int videoStreamIndex = dm.get_video_stream_index();
    auto frameRate = dm.get_frame_rate(videoStreamIndex);
    result.frameRate = frameRate.first / frameRate.second;
    //result.frameRate = (((double)1.0) / dm.get_duration(videoStreamIndex));
    pair<int,int> tb = dm.get_time_base(videoStreamIndex);
    result.timeBaseNum = tb.first;
    result.timeBaseDen = tb.second;

    int streamIndex = 0;
    while(dm.read_frame(streamIndex))
    {
        if(streamIndex != videoStreamIndex)
            continue;

        if(dm.is_key())
        {
            if(!foundFirstKey)
            {
                indexFirstKey = currentIndex;
                foundFirstKey = true;
            }
            else
            {
                if(!foundGOPSize)
                {
                    result.gopSize = currentIndex - indexFirstKey;
                    foundGOPSize = true;
                }
            }
        }

        r_packet pkt = dm.get();
 
        frameSizes.push_back((uint32_t)pkt.get_data_size());

        currentIndex++;
    }

    uint32_t sum = 0;
    sum = std::accumulate(begin(frameSizes), end(frameSizes), sum);

    uint32_t avgSize = sum / frameSizes.size();

    result.averageBitRate = (uint32_t)((avgSize * (frameRate.first / frameRate.second)) * 8);

    result.numFrames = currentIndex;

    return result;
}

void r_demuxer::_open_streams()
{
    if(avformat_open_input(&_context, _fileName.c_str(), nullptr, nullptr) < 0)
        R_STHROW(r_not_found_exception, ("Unable to open input file."));

    //_context->max_analyze_duration = 1;

    _streamTypes.clear();

    if(avformat_find_stream_info(_context, nullptr) >= 0)
    {
        for(int i = 0; i < (int)_context->nb_streams; i++)
        {
            if(_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN)
            {
                struct r_stream_info st = { STREAM_TYPE_UNKNOWN, i };
                _streamTypes.push_back(st);
            }
            else if(_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                struct r_stream_info st = { STREAM_TYPE_VIDEO, i };
                _streamTypes.push_back(st);

                if(_videoStreamIndex == STREAM_TYPE_UNKNOWN)
                    _videoStreamIndex = i;

				// if codec is not h.264, then applying our bitstream filter makes no sense.
                if(_bsf && (_context->streams[i]->codecpar->codec_id != AV_CODEC_ID_H264))
                {
                    
                    av_bsf_free(&_bsf);
                    _bsf = nullptr;
                }
            }
            else if(_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                struct r_stream_info st = { STREAM_TYPE_AUDIO, i };
                _streamTypes.push_back(st);

                if(_audioPrimaryStreamIndex == STREAM_TYPE_UNKNOWN)
                    _audioPrimaryStreamIndex = i;
            }
            else if(_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_DATA)
            {
                struct r_stream_info st = { STREAM_TYPE_DATA, i };
                _streamTypes.push_back(st);
            }
            else if(_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
            {
                struct r_stream_info st = { STREAM_TYPE_SUBTITLE, i };
                _streamTypes.push_back(st);
            }
            else if(_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_ATTACHMENT)
            {
                struct r_stream_info st = { STREAM_TYPE_ATTACHMENT, i };
                _streamTypes.push_back(st);
            }
        }
    }
    else R_STHROW(r_internal_exception, ("Unable to locate stream info."));

    if(_videoStreamIndex == -1)
        R_STHROW(r_internal_exception, ("Failed to find video stream in file."));

    _deMuxPkt.size = 0;
}

void r_demuxer::_open_custom_io_context(const uint8_t* buffer, size_t bufferSize)
{
    _storage.resize(bufferSize);
    memcpy(&_storage[0], buffer, bufferSize);

    _memoryIOContext = avio_alloc_context((uint8_t*)av_malloc(bufferSize), (int)bufferSize, 0, this, _read, nullptr, _seek);
    if(!_memoryIOContext)
        R_STHROW(r_internal_exception, ("Unable to allocate IO context."));

    _context = avformat_alloc_context();
    if(!_context)
        R_STHROW(r_internal_exception, ("Unable to allocate input IO context."));

    _context->pb = _memoryIOContext;
}

int r_demuxer::_read(void* opaque, uint8_t* dest, int size)
{
    r_demuxer* obj = (r_demuxer*)opaque;

    if((obj->_pos + size) > (int)obj->_storage.size())
        size = (int)(obj->_storage.size() - obj->_pos);

    memcpy(dest, &obj->_storage[0] + obj->_pos, size);
    obj->_pos += size;

    return size;
}

int64_t r_demuxer::_seek(void* opaque, int64_t offset, int whence)
{
    r_demuxer* obj = (r_demuxer*)opaque;

    if(whence == SEEK_CUR)
    {
        offset += obj->_pos;
        obj->_pos = offset;
        return obj->_pos;
    }
    else if(whence == SEEK_END)
    {
        offset += obj->_storage.size();
        obj->_pos = offset;
        return obj->_pos;
    }
    else if(whence == SEEK_SET)
    {
        obj->_pos = offset;
        return obj->_pos;
    }

    return -1;
}

void r_demuxer::_free_packet()
{
    if(_deMuxPkt.size > 0)
        av_packet_unref(&_deMuxPkt);
}

void r_demuxer::_free_filter_packet()
{
    if(_filterPkt.size > 0)
        av_packet_unref(&_filterPkt);
}

void r_demuxer::_init_annexb_filter()
{
    const AVBitStreamFilter *filter = av_bsf_get_by_name("h264_mp4toannexb");

    auto ret = av_bsf_alloc(filter, &_bsf);
    if(ret < 0)
        R_STHROW(r_internal_exception, ("Unable to av_bsf_alloc()"));

    auto st = _context->streams[get_video_stream_index()];

    ret = avcodec_parameters_copy(_bsf->par_in, st->codecpar);
    if (ret < 0)
        R_STHROW(r_internal_exception, ("Unable to avcodec_parameters_copy()"));

    ret = av_bsf_init(_bsf);
    if(ret < 0)
        R_STHROW(r_internal_exception, ("Unable to av_bsf_init()"));

    ret = avcodec_parameters_copy(st->codecpar, _bsf->par_out);
    if (ret < 0)
        R_STHROW(r_internal_exception, ("Unable to avcodec_parameters_copy()"));
}

void r_demuxer::_optional_annexb_filter()
{
    if(_bsf)
    {
        _free_filter_packet();

        auto ret = av_bsf_send_packet(_bsf, &_deMuxPkt);
        if (ret < 0)
            R_STHROW(r_internal_exception, ("Unable to av_bsf_send_packet()"));

        ret = av_bsf_receive_packet(_bsf, &_filterPkt);
        if (ret < 0)
            R_STHROW(r_internal_exception, ("Unable to av_bsf_receive_packet()"));
    }
}

void r_demuxer::_clear() noexcept
{
    _free_packet();
    _free_filter_packet();

    if(_context)
        avformat_close_input(&_context);

    if(_memoryIOContext)
    {
        av_freep(&_memoryIOContext->buffer);
        av_free(_memoryIOContext);
    }

    if(_bsf)
        av_bsf_free(&_bsf);
}