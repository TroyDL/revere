
#ifndef r_mux_r_demuxer_h
#define r_mux_r_demuxer_h

#include "r_mux/r_format_utils.h"
#include "r_utils/r_std_utils.h"

#include <string>
#include <vector>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/bsf.h"
}

namespace r_mux
{

enum r_demuxer_stream_type
{
    r_demuxer_STREAM_TYPE_UNKNOWN = -1,
    r_demuxer_STREAM_TYPE_VIDEO,
    r_demuxer_STREAM_TYPE_AUDIO,
};

struct r_stream_info
{
    AVCodecID codec_id;
    AVRational time_base;
    std::vector<uint8_t> extradata;
    int profile;
    int level;

    AVRational frame_rate;
    std::pair<uint16_t, uint16_t> resolution;

    uint8_t bits_per_raw_sample;
    uint8_t channels;
    uint32_t sample_rate;
};

struct r_frame_info
{
    int index;
    uint8_t* data;
    size_t size;
    int64_t pts;
    int64_t dts;
    bool key;
};

class r_demuxer final
{
public:
    r_demuxer(const std::string& fileName, bool annexBFilter = true);
    r_demuxer(const r_demuxer&) = delete;
    r_demuxer(r_demuxer&& obj);
    ~r_demuxer();

    r_demuxer& operator=(const r_demuxer&) = delete;
    r_demuxer& operator=(r_demuxer&& obj);

    int get_stream_count() const;
    int get_video_stream_index() const;
    int get_audio_stream_index() const;

    struct r_stream_info get_stream_info(int stream_index) const;

    std::vector<uint8_t> get_extradata(int stream_index) const;

    bool read_frame();
    struct r_frame_info get_frame_info() const;

private:
    void _free_demux_pkt();
    void _free_filter_pkt();
    void _init_annexb_filter();
    void _optional_annexb_filter();

    r_utils::r_std_utils::raii_ptr<AVFormatContext> _context;
    int _video_stream_index;
    int _audio_stream_index;
    AVPacket _demux_pkt;
    AVPacket _filter_pkt;
    r_utils::r_std_utils::raii_ptr<AVBSFContext> _bsf;

};

}

#endif
