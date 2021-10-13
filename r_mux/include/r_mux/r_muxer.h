
#ifndef r_mux_r_muxer_h
#define r_mux_r_muxer_h

#include "r_mux/r_format_utils.h"
#include "r_utils/r_std_utils.h"

#include <string>
#include <utility>
#include <cstdint>
#include <memory>
#include <functional>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/rational.h>
}

namespace r_mux
{

AVCodecID encoding_to_av_codec_id(const std::string& codec_name);

class r_muxer final
{
public:
    r_muxer(const std::string& path, bool output_to_buffer=false);
    r_muxer(const r_muxer&) = delete;
    r_muxer(r_muxer&& obj) = delete;
    ~r_muxer();

    r_muxer& operator=(const r_muxer&) = delete;
    r_muxer& operator=(r_muxer&&) = delete;

    void add_video_stream(AVRational frame_rate, AVCodecID codec_id, uint16_t w, uint16_t h, int profile, int level);
    void add_audio_stream(AVCodecID codec_id, uint8_t bits_per_raw_sample, uint8_t channels, uint32_t sample_rate);

    void set_video_extradata(const std::vector<uint8_t>& ed);
    void set_audio_extradata(const std::vector<uint8_t>& ed);

    void open();

    void write_video_frame(uint8_t* p, size_t size, int64_t input_pts, int64_t input_dts, AVRational input_time_base, bool key);
    void write_audio_frame(uint8_t* p, size_t size, int64_t input_pts, AVRational input_time_base);

    uint8_t* extradata();
    int extradata_size();

    void finalize();

    const uint8_t* buffer() const;
    size_t buffer_size() const;

private:
    std::string _path;
    bool _output_to_buffer;
    std::vector<uint8_t> _buffer;

    r_utils::r_std_utils::raii_ptr<AVFormatContext> _fc;
    AVStream* _video_stream; // raw pointers allowed here because they are cleaned up automatically by _fc
    AVStream* _audio_stream;
    bool _needs_finalize;
};

}

#endif
