
#include "r_mux/r_muxer.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_exception.h"
#include <stdexcept>

using namespace std;
using namespace r_mux;
using namespace r_utils;
using namespace r_utils::r_std_utils;

AVCodecID r_mux::encoding_to_av_codec_id(const string& codec_name)
{
    auto lower_codec_name = r_string_utils::to_lower(codec_name);
    if(lower_codec_name == "h264")
        return AV_CODEC_ID_H264;
    else if(lower_codec_name == "h265" || lower_codec_name == "hevc")
        return AV_CODEC_ID_HEVC;
    else if(lower_codec_name == "mp4a-latm")
        return AV_CODEC_ID_AAC_LATM;
    else if(lower_codec_name == "mpeg4-generic")
        return AV_CODEC_ID_AAC;
    else if(lower_codec_name == "pcmu")
        return AV_CODEC_ID_PCM_MULAW;

    R_THROW(("Unknown codec name."));
}

r_muxer::r_muxer(const std::string& path, bool output_to_buffer) :
    _path(path),
    _output_to_buffer(output_to_buffer),
    _buffer(),
    _fc([](AVFormatContext* fc){avformat_free_context(fc);}),
    _video_stream(nullptr),
    _audio_stream(nullptr),
    _needs_finalize(false)
{
    avformat_alloc_output_context2(&_fc.raw(), NULL, NULL, _path.c_str());
    if(!_fc)
        throw runtime_error("Unable to create libavformat output context");

    if(_fc.get()->oformat->flags & AVFMT_GLOBALHEADER)
        _fc.get()->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    _fc.get()->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
}

r_muxer::~r_muxer()
{
    if(_needs_finalize)
        finalize();
}

void r_muxer::add_video_stream(AVRational frame_rate, AVCodecID codec_id, uint16_t w, uint16_t h, int profile, int level)
{
    auto codec = avcodec_find_encoder(codec_id);
    if(!codec)
        throw runtime_error("Unable to find encoder stream.");

    _video_stream = avformat_new_stream(_fc.get(), codec);
    if(!_video_stream)
        throw runtime_error("Unable to allocate AVStream.");

    _video_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    _video_stream->codecpar->codec_id = codec_id;
    _video_stream->codecpar->width = w;
    _video_stream->codecpar->height = h;
    _video_stream->codecpar->format = AV_PIX_FMT_YUV420P;
    _video_stream->codecpar->profile = profile;
    _video_stream->codecpar->level = level;
    
    _video_stream->time_base.num = frame_rate.den;
    _video_stream->time_base.den = frame_rate.num;
}

void r_muxer::add_audio_stream(AVCodecID codec_id, uint8_t channels, uint32_t sample_rate)
{
    auto codec = avcodec_find_encoder(codec_id);

    _audio_stream = avformat_new_stream(_fc.get(), codec);
    if(!_audio_stream)
        throw runtime_error("Unable to allocate AVStream");

    _audio_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    _audio_stream->codecpar->codec_id = codec_id;
    //_audio_stream->codecpar->bits_per_raw_sample = bits_per_raw_sample;
    _audio_stream->codecpar->channels = channels;
    _audio_stream->codecpar->sample_rate = sample_rate;
    _audio_stream->time_base.num = 1;
    _audio_stream->time_base.den = sample_rate;
}

void r_muxer::set_video_extradata(const std::vector<uint8_t>& ed)
{
    if(!ed.empty())
    {
        if(_video_stream->codecpar->extradata)
            av_free(_video_stream->codecpar->extradata);

        _video_stream->codecpar->extradata = (uint8_t*)av_malloc(ed.size());
        memcpy(_video_stream->codecpar->extradata, &ed[0], ed.size());
        _video_stream->codecpar->extradata_size = (int)ed.size();
    }
}

void r_muxer::set_audio_extradata(const std::vector<uint8_t>& ed)
{
    if(!ed.empty())
    {
        if(_audio_stream->codecpar->extradata)
            av_free(_audio_stream->codecpar->extradata);

        _audio_stream->codecpar->extradata = (uint8_t*)av_malloc(ed.size());
        memcpy(_audio_stream->codecpar->extradata, &ed[0], ed.size());
        _audio_stream->codecpar->extradata_size = (int)ed.size();
    }
}

void r_muxer::open()
{
    if(_fc.get()->nb_streams < 1)
        throw runtime_error("Please add a stream before opening this muxer.");

    if(_output_to_buffer)
    {
        if(avio_open_dyn_buf(&_fc.get()->pb) < 0)
            throw runtime_error("Unable to allocate a memory IO object.");
    }
    else if(avio_open(&_fc.get()->pb, _path.c_str(), AVIO_FLAG_WRITE) < 0)
        throw runtime_error("Unable to open output io context.");

    if(avformat_write_header(_fc.get(), NULL) < 0)
        throw runtime_error("Unable to write header to output file.");

    _needs_finalize = true;
}

static void _get_packet_defaults(AVPacket* pkt)
{
    pkt->buf = nullptr;
    pkt->pts = AV_NOPTS_VALUE;
    pkt->dts = AV_NOPTS_VALUE;
    pkt->data = nullptr;
    pkt->size = 0;
    pkt->stream_index = 0;
    pkt->flags = 0;
    pkt->side_data = nullptr;
    pkt->side_data_elems = 0;
    pkt->duration = 0;
    pkt->pos = -1;
}

void r_muxer::write_video_frame(uint8_t* p, size_t size, int64_t input_pts, int64_t input_dts, AVRational input_time_base, bool key)
{
    if(_fc.get()->pb == nullptr)
        throw runtime_error("Please call open() before writing frames.");

    raii_ptr<AVPacket> pkt(av_packet_alloc(), [](AVPacket* pkt){av_packet_free(&pkt);});
    _get_packet_defaults(pkt.get());

    pkt.get()->stream_index = _video_stream->index;
    pkt.get()->data = p;
    pkt.get()->size = (int)size;
    pkt.get()->pts = av_rescale_q(input_pts, input_time_base, _video_stream->time_base);
    pkt.get()->dts = av_rescale_q(input_dts, input_time_base, _video_stream->time_base);
    pkt.get()->flags |= (key)?AV_PKT_FLAG_KEY:0;

    if(av_interleaved_write_frame(_fc.get(), pkt.get()) < 0)
        throw runtime_error("Unable to write frame to output stream.");
}

void r_muxer::write_audio_frame(uint8_t* p, size_t size, int64_t input_pts, AVRational input_time_base)
{
    if(_fc.get()->pb == nullptr)
        throw runtime_error("Please call open() before writing frames.");

    raii_ptr<AVPacket> pkt(av_packet_alloc(), [](AVPacket* pkt){av_packet_free(&pkt);});
    _get_packet_defaults(pkt.get());

    pkt.get()->stream_index = _audio_stream->index;
    pkt.get()->data = p;
    pkt.get()->size = (int)size;
    pkt.get()->pts = av_rescale_q(input_pts, input_time_base, _audio_stream->time_base);
    pkt.get()->dts = pkt.get()->pts;
    pkt.get()->flags = AV_PKT_FLAG_KEY;

    if(av_interleaved_write_frame(_fc.get(), pkt.get()) < 0)
        throw runtime_error("Unable to write frame to output stream.");
}

uint8_t* r_muxer::extradata()
{
    return _video_stream->codecpar->extradata;
}

int r_muxer::extradata_size()
{
    return _video_stream->codecpar->extradata_size;
}

void r_muxer::finalize()
{
    if(_needs_finalize)
    {
        _needs_finalize = false;
        av_write_trailer(_fc.get());

        if(_output_to_buffer)
        {
            raii_ptr<uint8_t> fileBytes([](uint8_t* p){av_freep(p);});
            int fileSize = avio_close_dyn_buf(_fc.get()->pb, &fileBytes.raw());
            _buffer.resize(fileSize);
            memcpy(&_buffer[0], fileBytes.get(), fileSize);
        }
        else avio_close(_fc.get()->pb);
    }
}

const uint8_t* r_muxer::buffer() const
{
    if(!_output_to_buffer)
        throw runtime_error("Please only call buffer() on muxers configured to output to buffer.");

    return &_buffer[0];
}

size_t r_muxer::buffer_size() const
{
    if(!_output_to_buffer)
        throw runtime_error("Please only call buffer_size() on muxers configured to output to buffer.");

    return _buffer.size();
}
