
#include "r_av/r_video_decoder.h"
#include "r_av/r_utils.h"
#include "r_av/r_locky.h"
#include "r_utils/r_string_utils.h"

extern "C"
{
#include "libavutil/opt.h"
#include "libavutil/frame.h"
}

using namespace r_av;
using namespace r_utils;
using namespace std;

r_video_decoder::r_video_decoder(r_av_codec_id codec_id, r_pix_fmt format, const r_codec_options& options) :
    _codecID(codec_id),
    _options(options),
    _pf(make_shared<r_packet_factory_default>()),
    _codec(nullptr),
    _context(nullptr),
    _frame(av_frame_alloc()),
    _scaler(nullptr),
    _outputWidth(0),
    _outputHeight(0),
    _format(format),
    _outputPkt(),
    _inputPkt()
{
    if(codec_id == r_av_codec_id_h264)
        _codec = avcodec_find_decoder_by_name("h264_mmal");

    if(!_codec)
        _codec = avcodec_find_decoder(r_av_codec_id_to_ffmpeg_codec_id(_codecID));
    
    if(!_codec)
        R_STHROW(r_internal_exception, ("Unable to find suitable codec."));
    
    _context = avcodec_alloc_context3(_codec);

    if(!_context)
        R_STHROW(r_internal_exception, ("Unable to allocate context."));

    if( !r_locky::is_registered() )
        R_STHROW(r_internal_exception, ( "Please call locky::register_ffmpeg() before using this class."));    

    if(!_frame)
        R_STHROW(r_internal_exception, ("Unable to allocate frame."));

    if(!_options.thread_count.is_null())
    {
        _context->thread_count = _options.thread_count.value();
    }

    _context->thread_type = FF_THREAD_FRAME;

    if(avcodec_open2(_context, _codec, nullptr) < 0)
        R_STHROW(r_internal_exception, ("Unable to open decoder."));
}

r_video_decoder::r_video_decoder(AVCodecParameters* codecpar, r_av_codec_id codec_id, r_pix_fmt format, const r_codec_options& options) :
    _codecID(codec_id),
    _options(options),
    _pf(make_shared<r_packet_factory_default>()),
    _codec(nullptr),
    _context(nullptr),
    _frame(av_frame_alloc()),
    _scaler(nullptr),
    _outputWidth(0),
    _outputHeight(0),
    _format(format),
    _outputPkt(),
    _inputPkt()
{
    if(codec_id == r_av_codec_id_h264)
        _codec = avcodec_find_decoder_by_name("h264_mmal");

    if(!_codec)
        _codec = avcodec_find_decoder(r_av_codec_id_to_ffmpeg_codec_id(_codecID));
    
    if(!_codec)
        R_STHROW(r_internal_exception, ("Unable to find suitable codec."));
    
    _context = avcodec_alloc_context3(_codec);

    if(!_context)
        R_STHROW(r_internal_exception, ("Unable to allocate context."));

    if( !r_locky::is_registered() )
        R_STHROW(r_internal_exception, ( "Please call locky::register_ffmpeg() before using this class."));

    if(!_frame)
        R_STHROW(r_internal_exception, ("Unable to allocate frame."));

    if(avcodec_parameters_to_context(_context, codecpar) != 0)
        R_STHROW(r_internal_exception, ("Unable to copy codec parameters."));

    if(!_options.thread_count.is_null())
    {
        _context->thread_count = _options.thread_count.value();
    }

    _context->thread_type = FF_THREAD_FRAME;

    if(avcodec_open2(_context, _codec, nullptr) < 0)
        R_STHROW(r_internal_exception, ("Unable to open decoder."));
}

r_video_decoder::r_video_decoder(r_video_decoder&& obj) noexcept
{
    _codecID = std::move(obj._codecID);
    obj._codecID = r_av_codec_id_h264;
    _options = std::move(obj._options);
    _pf = std::move(obj._pf);
    obj._pf.reset();
    _codec = std::move(obj._codec);
    obj._codec = nullptr;
    _context = std::move(obj._context);
    obj._context = nullptr;
    _frame = std::move(obj._frame);
    obj._frame = nullptr;
    _scaler = std::move(obj._scaler);
    obj._scaler = nullptr;
    _outputWidth = std::move(obj._outputWidth);
    obj._outputWidth = 0;
    _outputHeight = std::move(obj._outputHeight);
    obj._outputHeight = 0;
    _format = std::move(obj._format);
    obj._format = r_av_pix_fmt_yuv420p;
    _outputPkt = std::move(obj._outputPkt);
    obj._outputPkt = r_packet();
    _inputPkt = std::move(obj._inputPkt);
    obj._inputPkt = r_packet();
}

r_video_decoder::~r_video_decoder() noexcept
{
    _clear();
}

r_video_decoder& r_video_decoder::operator=(r_video_decoder&& obj) noexcept
{
    _clear();

    _codecID = std::move(obj._codecID);
    obj._codecID = r_av_codec_id_h264;
    _options = std::move(obj._options);
    _pf = std::move(obj._pf);
    obj._pf.reset();
    _codec = std::move(obj._codec);
    obj._codec = nullptr;
    _context = std::move(obj._context);
    obj._context = nullptr;
    _frame = std::move(obj._frame);
    obj._frame = nullptr;
    _scaler = std::move(obj._scaler);
    obj._scaler = nullptr;
    _outputWidth = std::move(obj._outputWidth);
    obj._outputWidth = 0;
    _outputHeight = std::move(obj._outputHeight);
    obj._outputHeight = 0;
    _format = std::move(obj._format);
    obj._format = r_av_pix_fmt_yuv420p;
    _outputPkt = std::move(obj._outputPkt);
    obj._outputPkt = r_packet();
    _inputPkt = std::move(obj._inputPkt);
    obj._inputPkt = r_packet();

    return *this;
}

r_video_decoder::r_decoder_state r_video_decoder::decode(const r_packet& pkt)
{
    _inputPkt = pkt;

    AVPacket inputPacket;
    av_init_packet( &inputPacket );

    if(_inputPkt.empty())
    {
        // flush packet
        inputPacket.data = nullptr;
        inputPacket.size = 0;
    }
    else
    {
        inputPacket.data = _inputPkt.map();
        inputPacket.size = (int)_inputPkt.get_data_size();
        inputPacket.pts = _inputPkt.get_pts();
        inputPacket.dts = _inputPkt.get_dts();
        inputPacket.duration = _inputPkt.get_duration();
    }

    auto sendResult = avcodec_send_packet(_context, &inputPacket);

    if(sendResult == AVERROR(EAGAIN))
    {
        auto receiveResult = avcodec_receive_frame(_context, _frame);

        if(receiveResult < 0)
            R_STHROW(r_internal_exception, ("Decoder is full, but no frames available?"))

        sendResult = avcodec_send_packet(_context, &inputPacket);
        if(sendResult != 0)
            R_STHROW(r_internal_exception, ("Just received a frame, but cannot send?"));
    }
    else if(sendResult == AVERROR_EOF)
    {
        auto receiveResult = avcodec_receive_frame(_context, _frame);

        if(receiveResult == AVERROR_EOF)
            return r_decoder_state_eof;

        if(receiveResult < 0)
            R_STHROW(r_internal_exception, ("Unable to receive frame."))
    }
    else if(sendResult == 0)
    {
        auto receiveResult = avcodec_receive_frame(_context, _frame);

        if(receiveResult == AVERROR_EOF)
            return r_decoder_state_eof;

        if(receiveResult == AVERROR(EAGAIN))
            return r_decoder_state_hungry;
    }
    else
    {
        char errbuf[4096];
        av_strerror(sendResult, errbuf, 4096);
        R_STHROW(r_internal_exception, ("Unknown error returned from avcodec_send_packet(): %s",errbuf));
    }

    _frame->pts = inputPacket.pts;

    return r_decoder_state_accepted;
}

void r_video_decoder::set_extradata(const std::vector<uint8_t>& ed)
{
    if(!_context)
        R_STHROW(r_internal_exception, ("Unable to set extradata on null decoder context."));

    auto edSize = ed.size();
    _context->extradata = (uint8_t*)av_malloc(edSize);
    if(!_context->extradata)
        R_STHROW(r_internal_exception, ("Failed to allocate extradata buffer."));
    _context->extradata_size = edSize;
    memcpy(_context->extradata, &ed[0], edSize);
}

void r_video_decoder::set_output_width(uint16_t w)
{
    if(_outputWidth != w)
    {
        _outputWidth = w;

        if(_scaler)
            _destroy_scaler();
    }
}

void r_video_decoder::set_output_height(uint16_t h)
{
    if(_outputHeight != h)
    {
        _outputHeight = h;

        if(_scaler)
            _destroy_scaler();
    }
}

r_packet r_video_decoder::get()
{
    if( _outputWidth == 0 )
        _outputWidth = _context->width;

    if( _outputHeight == 0 )
        _outputHeight = _context->height;

    auto pictureSize = picture_size(_format, _outputWidth, _outputHeight);
    _outputPkt = _pf->get( pictureSize );
    _outputPkt.migrate_md_from(_inputPkt);
    _outputPkt.set_data_size( pictureSize );
    _outputPkt.set_width( _outputWidth );
    _outputPkt.set_height( _outputHeight );

    if( _scaler == nullptr )
    {
        _scaler = sws_getContext( _context->width,
                                  _context->height,
                                  _context->pix_fmt,
                                  _outputWidth,
                                  _outputHeight,
                                  r_av_pix_fmt_to_ffmpeg_pix_fmt(_format),
                                  SWS_BICUBIC,
                                  nullptr,
                                  nullptr,
                                  nullptr );

        if( !_scaler )
            R_STHROW(r_internal_exception, ("Unable to allocate scaler context "
                                            "(input_width=%u,input_height=%u,output_width=%u,output_height=%u).",
                                            _context->width, _context->height, _outputWidth, _outputHeight ));
    }

    uint8_t* dest = _outputPkt.map();

    uint8_t* fields[3];
    int fieldWidths[3];

    if(_format == r_av_pix_fmt_rgba)
    {
        fields[0] = dest;
        fieldWidths[0] = _outputWidth * 4;
    }
    else
    {
        fields[0] = dest;
        dest += _outputWidth * _outputHeight;
        fields[1] = dest;
        dest += (_outputWidth/4) * _outputHeight;
        fields[2] = dest;

        fieldWidths[0] = _outputWidth;
        fieldWidths[1] = _outputWidth/2;
        fieldWidths[2] = fieldWidths[1];
    }

    int ret = sws_scale( _scaler,
                         _frame->data,
                         _frame->linesize,
                         0,
                         _context->height,
                         fields,
                         fieldWidths );
    if( ret <= 0 )
        R_STHROW(r_internal_exception, ( "Unable to create image." ));

    return _outputPkt;
}

void r_video_decoder::_destroy_scaler()
{
    if(_scaler)
    {
        sws_freeContext(_scaler);
        _scaler = nullptr;
    }
}

void r_video_decoder::_clear() noexcept
{
    _destroy_scaler();

    if(_frame)
        av_frame_free(&_frame);

    if(_context)
    {
        avcodec_close(_context);

        avcodec_free_context(&_context);
    }
}
