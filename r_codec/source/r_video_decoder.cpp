
#include "r_codec/r_video_decoder.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_std_utils.h"

using namespace r_codec;
using namespace r_utils;
using namespace r_utils::r_std_utils;
using namespace std;

static string _ff_rc_to_msg(int rc)
{
    char msg_buffer[1024];
    if(av_strerror(rc, msg_buffer, 1024) < 0)
    {
        throw runtime_error("Unknown ff return code.");
    }
    return string(msg_buffer);
}

#define S_FIELD_LT(a, b, c) if(a.c < b.c) return true

bool r_codec::operator<(const r_scaler_state& lhs, const r_scaler_state& rhs)
{
    S_FIELD_LT(lhs, rhs, input_format);
    S_FIELD_LT(lhs, rhs, input_width);
    S_FIELD_LT(lhs, rhs, input_height);
    S_FIELD_LT(lhs, rhs, output_format);
    S_FIELD_LT(lhs, rhs, output_width);
    S_FIELD_LT(lhs, rhs, output_height);
    return false;
}

r_video_decoder::r_video_decoder() :
    _codec_id(AV_CODEC_ID_NONE),
    _codec(nullptr),
    _context(nullptr),
    _parser(nullptr),
    _parse_input(false),
    _buffer(),
    _pos(nullptr),
    _remaining_size(0),
    _frame(nullptr),
    _scalers()
{
}

r_video_decoder::r_video_decoder(AVCodecID codec_id, bool parse_input) :
    _codec_id(codec_id),
    _codec(avcodec_find_decoder(_codec_id)),
    _context(avcodec_alloc_context3(_codec)),
    _parser(av_parser_init(_codec_id)),
    _parse_input(parse_input),
    _buffer(),
    _pos(nullptr),
    _remaining_size(0),
    _frame(av_frame_alloc()),
    _scalers()
{
    if(!_codec)
        R_THROW(("Failed to find codec"));
    if(!_context)
        R_THROW(("Failed to allocate context"));
    if(!_frame)
        R_THROW(("Failed to allocate frame"));

    if(!_parse_input)
        _parser->flags = PARSER_FLAG_COMPLETE_FRAMES;
    
    auto ret = avcodec_open2(_context, _codec, nullptr);
    if(ret < 0)
        R_THROW(("Failed to open codec: %s", _ff_rc_to_msg(ret).c_str()));
}

r_video_decoder::r_video_decoder(r_video_decoder&& obj) :
    _codec_id(std::move(obj._codec_id)),
    _codec(std::move(obj._codec)),
    _context(std::move(obj._context)),
    _parser(std::move(obj._parser)),
    _parse_input(std::move(obj._parse_input)),
    _buffer(std::move(obj._buffer)),
    _pos(std::move(obj._pos)),
    _remaining_size(std::move(obj._remaining_size)),
    _frame(std::move(obj._frame)),
    _scalers(std::move(obj._scalers))
{
    obj._codec_id = AV_CODEC_ID_NONE;
    obj._codec = nullptr;
    obj._context = nullptr;
    obj._parser = nullptr;
    obj._pos = nullptr;
    obj._frame = nullptr;
}

r_video_decoder::~r_video_decoder()
{
    _clear();
}

r_video_decoder& r_video_decoder::operator=(r_video_decoder&& obj)
{
    if(this != &obj)
    {
        _clear();

        _codec_id = std::move(obj._codec_id);
        obj._codec_id = AV_CODEC_ID_NONE;
        _codec = std::move(obj._codec);
        obj._codec = nullptr;
        _context = std::move(obj._context);
        obj._context = nullptr;
        _parser = std::move(obj._parser);
        obj._parser = nullptr;
        _parse_input = std::move(obj._parse_input);
        _buffer = std::move(obj._buffer);
        _pos = std::move(obj._pos);
        obj._pos = nullptr;
        _remaining_size = std::move(obj._remaining_size);
        _frame = std::move(obj._frame);
        obj._frame = nullptr;
        _scalers = std::move(obj._scalers);
    }

    return *this;
}

void r_video_decoder::attach_buffer(const uint8_t* data, size_t size)
{
    _buffer.resize(size);
    memcpy(_buffer.data(), data, size);
    _pos = _buffer.data();
    _remaining_size = _buffer.size();
}

r_video_decoder_state r_video_decoder::decode()
{
    raii_ptr<AVPacket> pkt(av_packet_alloc(), [](AVPacket* pkt){av_packet_free(&pkt);});

    int bytes_parsed = av_parser_parse2(_parser, _context, &pkt.get()->data, &pkt.get()->size, _pos, _remaining_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    if(bytes_parsed < 0)
        R_THROW(("Failed to parse data"));

    if(bytes_parsed == 0)
        return R_VIDEO_DECODER_STATE_HUNGRY;

    int sp_ret = avcodec_send_packet(_context, pkt.get());

    if(sp_ret == AVERROR(EAGAIN))
    {
        auto rf_ret = avcodec_receive_frame(_context, _frame);

        _pos += bytes_parsed;
        _remaining_size -= bytes_parsed;

        if(sp_ret < 0)
        {
            R_LOG_ERROR(("Failed to receive frame from decoder: %s", _ff_rc_to_msg(sp_ret).c_str()));
            return R_VIDEO_DECODER_STATE_HUNGRY;
        }

        if(rf_ret == 0)
            return R_VIDEO_DECODER_STATE_HAS_OUTPUT;
    }

    _pos += bytes_parsed;
    _remaining_size -= bytes_parsed;

    if(sp_ret < 0)
    {
        R_LOG_ERROR(("Failed to send packet to decoder: %s", _ff_rc_to_msg(sp_ret).c_str()));
        return R_VIDEO_DECODER_STATE_HUNGRY;
    }

    auto rf_ret = avcodec_receive_frame(_context, _frame);
    if(rf_ret == AVERROR(EAGAIN) || rf_ret == AVERROR_EOF)
        return R_VIDEO_DECODER_STATE_DECODE_AGAIN;
    else if(rf_ret < 0)
    {
        R_LOG_ERROR(("Failed to receive frame from decoder: %s", _ff_rc_to_msg(rf_ret).c_str()));
        return R_VIDEO_DECODER_STATE_HUNGRY;
    }
    else return R_VIDEO_DECODER_STATE_HAS_OUTPUT;
}

r_video_decoder_state r_video_decoder::flush()
{
FLUSH:
    int ret = avcodec_send_packet(_context, nullptr);
    if(ret < 0)
    {
        R_LOG_ERROR(("Failed to flush decoder: %s", _ff_rc_to_msg(ret).c_str()));
        return R_VIDEO_DECODER_STATE_HUNGRY;
    }
    else
    {
        auto rf_ret = avcodec_receive_frame(_context, _frame);
        if(rf_ret == AVERROR(EAGAIN))
            goto FLUSH;
        if(rf_ret == AVERROR_EOF)
            return R_VIDEO_DECODER_STATE_HUNGRY;
        else if(rf_ret < 0)
        {
            R_LOG_ERROR(("Failed to flush decoder: %s", _ff_rc_to_msg(rf_ret).c_str()));
            return R_VIDEO_DECODER_STATE_HUNGRY;
        }
        return R_VIDEO_DECODER_STATE_HAS_OUTPUT;
    }
}

vector<uint8_t> r_video_decoder::get(AVPixelFormat output_format, uint16_t output_width, uint16_t output_height)
{
    r_scaler_state state;
    state.input_format = _context->pix_fmt;
    state.input_width = _context->width;
    state.input_height = _context->height;
    state.output_format = output_format;
    state.output_width = output_width;
    state.output_height = output_height;

    auto found = _scalers.find(state);

    if(found == _scalers.end())
    {
        _scalers[state] = sws_getContext(
            _context->width,
            _context->height,
            _context->pix_fmt,
            output_width,
            output_height,
            output_format,
            SWS_BICUBIC,
            NULL,
            NULL,
            NULL
        );
    }

    auto output_image_size = av_image_get_buffer_size(output_format, output_width, output_height, 32);

    vector<uint8_t> result(output_image_size);

    uint8_t* fields[AV_NUM_DATA_POINTERS];
    int linesizes[AV_NUM_DATA_POINTERS];

    auto ret = av_image_fill_arrays(fields, linesizes, &result[0], output_format, output_width, output_height, 1);

    if(ret < 0)
        R_THROW(("Failed to fill arrays for picture: %s", _ff_rc_to_msg(ret).c_str()));

    ret = sws_scale(_scalers[state],
                    _frame->data,
                    _frame->linesize,
                    0,
                    _context->height,
                    fields,
                    linesizes);

    if(ret < 0)
        R_THROW(("sws_scale() failed: %s", _ff_rc_to_msg(ret).c_str()));

    return result;
}

uint16_t r_video_decoder::input_width() const
{
    return (uint16_t)_context->width;
}

uint16_t r_video_decoder::input_height() const
{
    return (uint16_t)_context->height;
}

void r_video_decoder::_clear()
{
    for(auto s : _scalers)
        sws_freeContext(s.second);
    _scalers.clear();

    if(_frame)
    {
        av_frame_free(&_frame);
        _frame = nullptr;
    }
    if(_parser)
    {
        av_parser_close(_parser);
        _parser = nullptr;
    }
    if(_context)
    {
        avcodec_close(_context);
        av_free(_context);
        _context = nullptr;
    }
}
