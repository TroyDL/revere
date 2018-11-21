
#ifndef __r_av_r_video_encoder_h
#define __r_av_r_video_encoder_h

#include "r_av/r_packet.h"
#include "r_av/r_packet_factory.h"
#include "r_av/r_options.h"
#include <vector>

extern "C"
{
#include "libavcodec/avcodec.h"
}

namespace r_av
{

class r_video_encoder final
{
public:
    enum r_encoder_state
    {
        r_encoder_state_accepted,
        r_encoder_state_hungry,
        r_encoder_state_eof
    };

    enum r_encoder_packet_type
    {
        r_encoder_packet_type_i,
        r_encoder_packet_type_p,
        r_encoder_packet_type_auto
    };

    r_video_encoder(r_av_codec_id codec_id, r_pix_fmt format, uint16_t w, uint16_t h, uint16_t gop_size, uint32_t bit_rate, uint32_t time_base_num, uint32_t time_base_den, const r_codec_options& options);
    r_video_encoder(const r_video_encoder&) = delete;
    r_video_encoder(r_video_encoder&& obj) noexcept;

    ~r_video_encoder() noexcept;

    r_video_encoder& operator=(const r_video_encoder&) = delete;
    r_video_encoder& operator=(r_video_encoder&& obj) noexcept;

    void set_packet_factory(std::shared_ptr<r_packet_factory> pf) { _pf = pf; }

    r_encoder_state encode_image(const r_packet& input, r_encoder_packet_type pt = r_encoder_packet_type_auto);

    r_packet get() { return _output; }

    std::vector<uint8_t> get_extradata() const { return _extraData; }
    std::pair<int, int> get_time_base() const { return std::make_pair((int)_context->time_base.num, (int)_context->time_base.den); }

private:
    void _clear() noexcept;

    r_av_codec_id _codecID;
    r_pix_fmt _pixFormat;
    r_codec_options _options;
    std::shared_ptr<r_packet_factory> _pf;
    AVCodec* _codec;
    AVCodecContext* _context;
    std::vector<uint8_t> _extraData;
    r_packet _output;
};

}

#endif
