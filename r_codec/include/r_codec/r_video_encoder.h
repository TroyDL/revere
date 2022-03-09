
#ifndef r_codec_r_video_encoder_h
#define r_codec_r_video_encoder_h

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
}

#include "r_codec/r_codec_state.h"
#include "r_utils/r_std_utils.h"

#include <vector>
#include <cstdint>
#include <map>

namespace r_codec
{

struct r_packet_info
{
    uint8_t* data;
    size_t size;
    int64_t pts;
    int64_t dts;
    int64_t duration;
    bool key;
    AVRational time_base;
};

class r_video_encoder final
{
public:
    r_video_encoder();

    r_video_encoder(
        AVCodecID codec_id,
        uint32_t bit_rate,
        uint16_t w,
        uint16_t h,
        AVRational framerate,
        AVPixelFormat pix_fmt,
        uint8_t max_b_frames,
        uint16_t gop_size,
        int profile,
        int level
    );

    r_video_encoder(const r_video_encoder&) = delete;

    r_video_encoder(r_video_encoder&& obj);

    ~r_video_encoder();

    r_video_encoder& operator=(const r_video_encoder&) = delete;
    r_video_encoder& operator=(r_video_encoder&& obj);

    void attach_buffer(const uint8_t* data, size_t size);

    r_codec_state encode();

    r_packet_info get();

    std::vector<uint8_t> get_extradata() const;

private:
    void _clear();

    AVCodecID _codec_id;
    const AVCodec* _codec;
    AVCodecContext* _context;
    int64_t _pts;
    std::vector<uint8_t> _buffer;
    r_utils::r_std_utils::raii_ptr<AVPacket> _pkt;
};

}

#endif