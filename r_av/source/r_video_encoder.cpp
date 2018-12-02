
#include "r_av/r_video_encoder.h"
#include "r_av/r_utils.h"
#include "r_av/r_locky.h"
#include "r_utils/r_string_utils.h"

using namespace r_av;
using namespace r_utils;
using namespace std;

r_video_encoder::r_video_encoder(r_av_codec_id codec_id, r_pix_fmt format, uint16_t w, uint16_t h, uint16_t gop_size, uint32_t bit_rate, uint32_t time_base_num, uint32_t time_base_den, const r_codec_options& options) :
    _codecID(codec_id),
    _pixFormat(format),
    _options(options),
    _pf(std::make_shared<r_packet_factory_default>()),
    _codec(avcodec_find_encoder(r_av_codec_id_to_ffmpeg_codec_id(_codecID))),
    _context(avcodec_alloc_context3(_codec)),
    _extraData(),
    _output()
{
    if(!r_locky::is_registered())
        R_STHROW(r_internal_exception, ( "Please call locky::register_ffmpeg() before using this class."));

    if(!_codec)
        R_STHROW(r_internal_exception, ("Unable to find codec."));

    if(!_context)
        R_STHROW(r_internal_exception, ("Unable to allocate context."));

    // massive amount of option setting here...
    _context->codec_id = r_av_codec_id_to_ffmpeg_codec_id(_codecID);
    _context->codec_type = AVMEDIA_TYPE_VIDEO;
    
    // XXX example transcoder shows pix_fmt coming from either _codec->pix_fmts[] OR decoders pix_fmt!
    _context->pix_fmt = (_codec->pix_fmts)?_codec->pix_fmts[0]:r_av_pix_fmt_to_ffmpeg_pix_fmt(_pixFormat);
    _context->width = w;
    _context->height = h;
    _context->gop_size = gop_size;
    _context->bit_rate = bit_rate;
    _context->time_base.num = time_base_num;
    _context->time_base.den = time_base_den;
    _context->max_b_frames = 0;
    //_context->qmin = 2;
    //_context->qmax = 31;

    _context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; //   XXX depends on muxer->format->oformat->flags

    if(!_options.profile.is_null() && _options.profile.value().length() > 0)
    {
        if(r_string_utils::to_lower(_options.profile.value()) == "baseline")
            _context->profile = FF_PROFILE_H264_BASELINE;
        else if(r_string_utils::to_lower(_options.profile.value()) == "main")
            _context->profile = FF_PROFILE_H264_MAIN;
        else if(r_string_utils::to_lower(_options.profile.value()) == "high")
            _context->profile = FF_PROFILE_H264_HIGH;

        av_opt_set( _context->priv_data, "profile", r_string_utils::to_lower(_options.profile.value()).c_str(), 0 );
    }

    if( !_options.preset.is_null() && _options.preset.value().length() > 0)
        av_opt_set( _context->priv_data, "preset", _options.preset.value().c_str(), 0 );

    if( !_options.tune.is_null() && _options.tune.value().length() > 0)
        av_opt_set( _context->priv_data, "tune", _options.tune.value().c_str(), 0 );

    if( !_options.thread_count.is_null() )
    {
        _context->thread_count = _options.thread_count.value();
    }

    _context->thread_type = FF_THREAD_FRAME;

    // We set this here to force FFMPEG to populate the extradata field (necessary if the output stream
    // is going to be muxed into a format with a global header (for example, mp4)).
    _context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if(avcodec_open2(_context, _codec, nullptr) < 0)
        R_STHROW(r_internal_exception, ("Failed to open encoding context."));

    if(_context->extradata)
    {
        _extraData.resize(_context->extradata_size);
        memcpy(&_extraData[0], _context->extradata, _context->extradata_size);
    }
}

r_video_encoder::r_video_encoder(r_video_encoder&& obj) noexcept
{
    _codecID = std::move(obj._codecID);
    _pixFormat = std::move(obj._pixFormat);
    _options = std::move(obj._options);
    _pf = std::move(obj._pf);
    obj._pf.reset();
    _codec = std::move(obj._codec);
    obj._codec = nullptr;
    _context = std::move(obj._context);
    obj._context = nullptr;
    _extraData = std::move(obj._extraData);
    _output = std::move(obj._output);
}

r_video_encoder::~r_video_encoder() noexcept
{
    _clear();
}

r_video_encoder& r_video_encoder::operator=(r_video_encoder&& obj) noexcept
{
    _clear();

    _codecID = std::move(obj._codecID);
    _pixFormat = std::move(obj._pixFormat);
    _options = std::move(obj._options);
    _pf = std::move(obj._pf);
    obj._pf.reset();
    _codec = std::move(obj._codec);
    obj._codec = nullptr;
    _context = std::move(obj._context);
    obj._context = nullptr;
    _extraData = std::move(obj._extraData);
    _output = std::move(obj._output);

    return *this;
}

r_video_encoder::r_encoder_state r_video_encoder::encode_image(const r_packet& input, r_encoder_packet_type pt)
{
    if(_pixFormat != r_av_pix_fmt_yuv420p && _pixFormat != r_av_pix_fmt_yuvj420p)
        R_STHROW(r_internal_exception, ("Encoder only supports YUV420P and YUVJ420P input currently."));

    AVFrame* frame = av_frame_alloc();
    if(!frame)
        R_STHROW(r_internal_exception, ("Unable to allocate AVFrame!"));

    frame->format = r_av_pix_fmt_to_ffmpeg_pix_fmt(_pixFormat);
    frame->width = _context->width;
    frame->height = _context->height;

    frame->pts = input.get_pts();

    if(!input.empty())
    {
        uint8_t* pic = input.map();

        frame->data[0] = pic;
        pic += _context->width * _context->height;
        frame->data[1] = pic;
        pic += ((_context->width/4) * _context->height);
        frame->data[2] = pic;

        frame->linesize[0] = _context->width;
        frame->linesize[1] = _context->width / 2;
        frame->linesize[2] = frame->linesize[1];
    }

    if(pt != r_encoder_packet_type_auto)
        frame->pict_type = (pt == r_encoder_packet_type_i) ? AV_PICTURE_TYPE_I : AV_PICTURE_TYPE_P;

    AVPacket* pkt = av_packet_alloc();
    if(!pkt)
        R_STHROW(r_internal_exception, ("Unable to allocate packet."));

    auto sendResult = avcodec_send_frame(_context, (!input.empty())?frame:nullptr);
    if(sendResult == AVERROR(EAGAIN))
    {
        auto receiveResult = avcodec_receive_packet(_context, pkt);
        if(receiveResult == AVERROR(EAGAIN))
            R_STHROW(r_internal_exception, ("Cant receive but also cant send?"));
        if(receiveResult < 0)
            R_STHROW(r_internal_exception, ("Unknown error in avcodec_receive_packet()."));

        sendResult = avcodec_send_frame(_context, frame);
        if(sendResult < 0)
            R_STHROW(r_internal_exception, ("Made room but still cant send frame!"));
    }
    else if(sendResult == AVERROR_EOF)
    {
        auto receiveResult = avcodec_receive_packet(_context, pkt);
        if(receiveResult == AVERROR_EOF)
        {
            av_frame_free(&frame);
            av_packet_free(&pkt);
            return r_encoder_state_eof;
        }
        if(receiveResult < 0)
            R_STHROW(r_internal_exception, ("Encoder flushed and unable to receive packet."));        
    }
    else if(sendResult == 0)
    {
        auto receiveResult = avcodec_receive_packet(_context, pkt);
        if(receiveResult == AVERROR(EAGAIN))
        {
            av_frame_free(&frame);
            av_packet_free(&pkt);
            return r_encoder_state_hungry;
        }
        if(receiveResult < 0)
            R_STHROW(r_internal_exception, ("Unknown error in avcodec_receive_packet."));    
    }

    av_frame_free(&frame);

    bool key = pkt->flags & AV_PKT_FLAG_KEY;

    _output = _pf->get(pkt->size);
    _output.set_key(key);
    _output.set_pts(pkt->pts);
    _output.set_dts(pkt->dts);
    _output.set_duration(pkt->duration);
    _output.set_time_base(get_time_base());

    uint8_t* writer = _output.map();
 
    memcpy(writer, pkt->data, pkt->size);
    _output.set_width(_context->width);
    _output.set_height(_context->height);
    _output.set_data_size(pkt->size);

    av_packet_free(&pkt);

    return r_encoder_state_accepted;
}

void r_video_encoder::_clear() noexcept
{
    if(_context)
    {
        avcodec_close(_context);
        av_free(_context);
    }
}
