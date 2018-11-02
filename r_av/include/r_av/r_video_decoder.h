
#ifndef __r_av_r_video_decoder_h
#define __r_av_r_video_decoder_h

#include "r_av/r_packet.h"
#include "r_av/r_packet_factory.h"
#include "r_av/r_options.h"
#include "r_av/r_demuxer.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

namespace r_av
{

class r_video_decoder final
{
public:
    enum r_decoder_state
    {
        r_decoder_state_accepted,
        r_decoder_state_hungry,
        r_decoder_state_eof
    };

    r_video_decoder(r_av_codec_id codec_id, r_pix_fmt format, const r_codec_options& options);
    r_video_decoder(AVCodecParameters* codecpar, r_av_codec_id codec_id, r_pix_fmt format, const r_codec_options& options);
    r_video_decoder(const r_video_decoder&) = delete;
    r_video_decoder(r_video_decoder&& obj) noexcept;

    ~r_video_decoder() noexcept;

    r_video_decoder& operator=(const r_video_decoder&) = delete;
    r_video_decoder& operator=(r_video_decoder&& obj) noexcept;

    void set_packet_factory(std::shared_ptr<r_packet_factory> pf) { _pf = pf; }

    r_decoder_state decode(const r_packet& pkt);

    void set_extradata(const std::vector<uint8_t>& ed);

    uint16_t get_input_width() const { return (uint16_t)_context->width; }
    uint16_t get_input_height() const { return (uint16_t)_context->height; }
    void set_output_width(uint16_t w);
    uint16_t get_output_width() const { return _outputWidth; }
    void set_output_height(uint16_t h);
    uint16_t get_output_height() const { return _outputHeight; }

    r_packet get();

private:
    void _destroy_scaler();
    void _clear() noexcept;

    r_av_codec_id _codecID;
    r_codec_options _options;
    std::shared_ptr<r_packet_factory> _pf;
    AVCodec* _codec;
    AVCodecContext* _context;
    AVFrame* _frame;
    SwsContext* _scaler;
    uint16_t _outputWidth;
    uint16_t _outputHeight;
    r_pix_fmt _format;
};

}

#endif
