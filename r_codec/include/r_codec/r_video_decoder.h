
#ifndef r_codec_r_video_decoder_h
#define r_codec_r_video_decoder_h

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
}

#include <vector>
#include <cstdint>
#include <map>

namespace r_codec
{

enum r_video_decoder_state
{
    R_VIDEO_DECODER_STATE_INITIALIZED,
    R_VIDEO_DECODER_STATE_HUNGRY,
    R_VIDEO_DECODER_STATE_DECODE_AGAIN,
    R_VIDEO_DECODER_STATE_HAS_OUTPUT
};

struct r_scaler_state
{
    AVPixelFormat input_format;
    uint16_t input_width;
    uint16_t input_height;
    AVPixelFormat output_format;
    uint16_t output_width;
    uint16_t output_height;
};

bool operator<(const r_scaler_state& lhs, const r_scaler_state& rhs);

class r_video_decoder
{
public:
    r_video_decoder();
    r_video_decoder(AVCodecID codec_id, bool parse_input = false);
    r_video_decoder(const r_video_decoder&) = delete;
    r_video_decoder(r_video_decoder&& obj);
    ~r_video_decoder();

    r_video_decoder& operator=(const r_video_decoder&) = delete;
    r_video_decoder& operator=(r_video_decoder&& obj);

    void set_extradata(const uint8_t* data, size_t size);

    void attach_buffer(const uint8_t* data, size_t size);

    r_video_decoder_state decode();
    r_video_decoder_state flush();

    std::vector<uint8_t> get(AVPixelFormat output_format, uint16_t output_width, uint16_t output_height);

    uint16_t input_width() const;
    uint16_t input_height() const;

private:
    void _clear();

    AVCodecID _codec_id;
    AVCodec* _codec;
    AVCodecContext* _context;
    AVCodecParserContext* _parser;
    bool _parse_input;
    std::vector<uint8_t> _buffer;
    uint8_t* _pos;
    int _remaining_size;
    AVFrame* _frame;
    std::map<r_scaler_state, SwsContext*> _scalers;
};

}

#endif