
#include "r_av/r_muxer.h"
#include "r_av/r_locky.h"
#include "r_av/r_utils.h"

extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
}

using namespace r_av;
using namespace r_utils;
using namespace std;

r_muxer::r_muxer(OUTPUT_LOCATION location, const string& fileName) :
    _location(location),
    _fileName(fileName),
    _streams(),
    _format(nullptr),
    _fc(nullptr),
    _ts(0),
    _isTS(r_string_utils::ends_with(r_string_utils::to_lower(_fileName), ".ts")),
    _isMP4(r_string_utils::ends_with(r_string_utils::to_lower(_fileName), ".mp4")),
    _oweTrailer(false),
    _numVideoFramesWritten(0),
    _fileNum(0)
{
    if( !r_locky::is_registered() )
        R_STHROW(r_internal_exception, ( "Please call locky::register_ffmpeg() before using this class."));

    if(r_string_utils::contains(_fileName, "rtp://"))
    {
        _fc = avformat_alloc_context();
        if(!_fc)
            R_STHROW(r_internal_exception, ("Unable to alloc output context."));

        _format = av_guess_format("rtp", nullptr, nullptr);

        _fc->oformat = _format;
    }
    else
    {
        avformat_alloc_output_context2(&_fc, nullptr, nullptr, _fileName.c_str());
        if(!_fc)
            R_STHROW(r_internal_exception, ("Unable to alloc output context."));

        _format = _fc->oformat;
    }

    if(_fc->oformat->flags & AVFMT_GLOBALHEADER)
        _fc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

r_muxer::r_muxer(r_muxer&& obj) noexcept
{
    _location = std::move(obj._location);
    obj._location = OUTPUT_LOCATION_FILE;
    _fileName = std::move(obj._fileName);
    obj._fileName = string();
    _streams = std::move(obj._streams);
    obj._streams.clear();
    _format = std::move(obj._format);
    obj._format = nullptr;
    _fc = std::move(obj._fc);
    obj._fc = nullptr;
    _ts = std::move(obj._ts);
    obj._ts = 0;
    _isTS = std::move(obj._isTS);
    obj._isTS = false;
    _oweTrailer = std::move(obj._oweTrailer);
    obj._oweTrailer = false;
    _numVideoFramesWritten = std::move(obj._numVideoFramesWritten);
    obj._numVideoFramesWritten = 0;
    _fileNum = std::move(obj._fileNum);
    obj._fileNum = 0;
}

r_muxer::~r_muxer() noexcept
{
    _clear();
}

r_muxer& r_muxer::operator=(r_muxer&& obj) noexcept
{
    _clear();

    _location = std::move(obj._location);
    obj._location = OUTPUT_LOCATION_FILE;
    _fileName = std::move(obj._fileName);
    obj._fileName = string();
    _streams = std::move(obj._streams);
    obj._streams.clear();
    _format = std::move(obj._format);
    obj._format = nullptr;
    _fc = std::move(obj._fc);
    obj._fc = nullptr;
    _ts = std::move(obj._ts);
    obj._ts = 0;
    _isTS = std::move(obj._isTS);
    obj._isTS = false;
    _oweTrailer = std::move(obj._oweTrailer);
    obj._oweTrailer = false;
    _numVideoFramesWritten = std::move(obj._numVideoFramesWritten);
    obj._numVideoFramesWritten = 0;
    _fileNum = std::move(obj._fileNum);
    obj._fileNum = 0;

    return *this;
}

int r_muxer::add_stream(const r_stream_options& soptions)
{
    r_muxer_stream stream;
    stream.options = soptions;

    if(stream.options.type.is_null())
        R_STHROW(r_internal_exception, ("Required option \"type\" missing."));

    if(stream.options.type.value() == "video")
    {
        if(stream.options.codec_id.is_null())
            R_STHROW(r_internal_exception, ("Required option \"codec_id\" missing."));
        if(stream.options.time_base_num.is_null())
            R_STHROW(r_internal_exception, ("Required option \"time_base_num\" missing."));
        if(stream.options.time_base_den.is_null())
            R_STHROW(r_internal_exception, ("Required option \"time_base_den\" missing."));
        if(stream.options.frame_rate_num.is_null())
            R_STHROW(r_internal_exception, ("Required option \"frame_rate_num\" missing."));
        if(stream.options.frame_rate_den.is_null())
            R_STHROW(r_internal_exception, ("Required option \"frame_rate_den\" missing."));
        if(stream.options.width.is_null())
            R_STHROW(r_internal_exception, ("Required option \"width\" missing."));
        if(stream.options.height.is_null())
            R_STHROW(r_internal_exception, ("Required option \"height\" missing."));

        auto ff_codec_id = r_av::r_av_codec_id_to_ffmpeg_codec_id((r_av_codec_id)stream.options.codec_id.value());

        auto codec = avcodec_find_encoder(ff_codec_id);
        if(!codec)
            R_STHROW(r_internal_exception, ("Could not find suitable encoder."));

        stream.st = avformat_new_stream(_fc, codec);

        stream.st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
        stream.st->codecpar->codec_id = ff_codec_id;
        stream.st->codecpar->width = stream.options.width.value();
        stream.st->codecpar->height = stream.options.height.value();
        stream.st->time_base.num = stream.options.time_base_num.value();
        stream.st->time_base.den = stream.options.time_base_den.value();
        stream.st->r_frame_rate.num = stream.options.frame_rate_num.value();
        stream.st->r_frame_rate.den = stream.options.frame_rate_den.value();

        if(!stream.options.format.is_null())
            stream.st->codecpar->format = (AVPixelFormat)r_av::r_av_pix_fmt_to_ffmpeg_pix_fmt((r_pix_fmt)stream.options.format.value());

        if(!stream.options.profile.is_null())
        {
            if(stream.options.profile.value() == "baseline")
                stream.st->codecpar->profile = FF_PROFILE_H264_BASELINE;
            else if(stream.options.profile.value() == "main")
                stream.st->codecpar->profile = FF_PROFILE_H264_MAIN;
            else if(stream.options.profile.value() == "high")
                stream.st->codecpar->profile = FF_PROFILE_H264_HIGH;
            else R_THROW(("Unknown profile."));
        }
    }
    else if(stream.options.type.value() == "audio")
    {
        if(stream.options.bits_per_raw_sample.is_null())
            R_STHROW(r_internal_exception, ("Required option \"bits_per_raw_sample\" missing."));
        if(stream.options.channels.is_null())
            R_STHROW(r_internal_exception, ("Required option \"channels\" missing."));
        if(stream.options.sample_rate.is_null())
            R_STHROW(r_internal_exception, ("Required option \"sample_rate\" missing."));

        auto ff_codec_id = r_av::r_av_codec_id_to_ffmpeg_codec_id((r_av_codec_id)stream.options.codec_id.value());

        auto codec = avcodec_find_encoder(ff_codec_id);
        if(!codec)
            R_STHROW(r_internal_exception, ("Could not find suitable encoder."));

        stream.st = avformat_new_stream(_fc, codec);

        stream.st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
        stream.st->codecpar->codec_id = ff_codec_id;
        stream.st->codecpar->bits_per_raw_sample = stream.options.bits_per_raw_sample.value();
        stream.st->codecpar->channels = stream.options.channels.value();
        stream.st->codecpar->sample_rate = stream.options.sample_rate.value();
    }

    auto index = _streams.size();
    _streams.push_back(stream);
    return index;
}

void r_muxer::set_extradata(const std::vector<uint8_t>& ed, int streamIndex)
{
    if(streamIndex >= (int)_streams.size())
        R_STHROW(r_internal_exception, ("Invalid stream index."));

    if( !(_fc->oformat->flags & AVFMT_GLOBALHEADER) )
        R_LOG_INFO("Extradata not required for %s container.",_fileName.c_str());
    else
    {
        _streams[streamIndex].st->codecpar->extradata = (uint8_t*)av_mallocz( ed.size() );
        if( !_streams[streamIndex].st->codecpar->extradata )
            R_STHROW(r_internal_exception, ("Unable to allocate extradata storage."));
        _streams[streamIndex].st->codecpar->extradata_size = (int)ed.size();

        memcpy( _streams[streamIndex].st->codecpar->extradata, &ed[0], ed.size() );
    }
}

void r_muxer::write_packet(const r_packet& input, int streamIndex, bool keyFrame)
{
    if(streamIndex >= (int)_streams.size())
        R_STHROW(r_internal_exception, ("Invalid stream index."));

    if(_fc->pb == nullptr)
        _open_io();

    if(_isTS)
    {
        if(_numVideoFramesWritten == 0)
        {
            if(_fileNum == 0)
                _write_header();
        }

        av_opt_set(_fc->priv_data, "mpegts_flags", "resend_headers", 0);
    }
    else
    {
        if(!_oweTrailer)
        {
            _write_header();
            _oweTrailer = true;
        }
    }

    AVPacket pkt;
    av_init_packet(&pkt);

    pkt.stream_index = _streams[streamIndex].st->index;
    pkt.data = input.map();
    pkt.size = (int)input.get_data_size();
    pkt.pts = _ts;
    pkt.dts = _ts;

    // This is our input time_base (a fraction describing how many pts ticks are in
    // one second of our inputs ts units).
    AVRational tb;
    tb.num = 1;
    tb.den = input.get_ts_freq();

    // convert our input timestamp from its timebase into our output timebase and then
    // increment our _ts by that much
    _ts += av_rescale_q(input.get_duration(), tb, _streams[streamIndex].st->time_base);

    pkt.flags |= (keyFrame) ? AV_PKT_FLAG_KEY : 0;

    if( av_interleaved_write_frame( _fc, &pkt ) < 0 )
        R_STHROW(r_internal_exception, ("Unable to write video frame."));

    ++_numVideoFramesWritten;
}

void r_muxer::finalize_file()
{
    if(_location != OUTPUT_LOCATION_FILE)
        R_STHROW(r_internal_exception, ("Cannot call finalize_file() on non file muxer."));

    _write_trailer();

    avio_close(_fc->pb);
    _fc->pb = nullptr;
}

vector<uint8_t> r_muxer::finalize_buffer()
{
    if(_location != OUTPUT_LOCATION_BUFFER)
        R_STHROW(r_internal_exception, ("Cannot get output buffer from non buffer muxer."));

    _write_trailer();

    uint8_t* fileBytes = nullptr;
    int fileSize = avio_close_dyn_buf(_fc->pb, &fileBytes);
    _fc->pb = nullptr;

    vector<uint8_t> buffer(fileSize);
    memcpy(&buffer[0], fileBytes, fileSize);

    av_freep(&fileBytes);

    return buffer;
}

void r_muxer::_write_header()
{
    AVDictionary* opts = NULL;
    if(_isMP4)
        av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov", 0);

    if(avformat_write_header(_fc, (_isMP4)?&opts:nullptr) != 0)
        R_STHROW(r_internal_exception, ("Unable to write header."));
}

void r_muxer::_write_trailer()
{
    if( !_fc->pb )
        R_STHROW(r_internal_exception, ("Unable to finalize an unopened IO object."));

    if(!_isTS && _oweTrailer)
    {
        av_write_trailer(_fc);
        _oweTrailer = false;
    }

    _fileNum++;

    _numVideoFramesWritten = 0;
}

void r_muxer::_open_io()
{
    if(_location == OUTPUT_LOCATION_BUFFER)
    {
        avio_open_dyn_buf(&_fc->pb);
        if(!_fc->pb)
            R_STHROW(r_internal_exception, ("Unable to allocation memory io object."));
    }
    else
    {
        // files and rtp are opened like this!
        if( avio_open( &_fc->pb, _fileName.c_str(), AVIO_FLAG_WRITE ) < 0 )
            R_STHROW(r_internal_exception, ("Unable to open output file."));
    }
}

void r_muxer::_clear() noexcept
{
    if(_isTS)
    {
        if(_fc)
        {
            if(_fc->pb == nullptr)
                _open_io();
            av_write_trailer(_fc);
        }
    }
    else if(_oweTrailer)
        _write_trailer();

    if(_fc && _fc->pb)
    {
        if(_location == OUTPUT_LOCATION_BUFFER)
        {
            uint8_t* fileBytes = nullptr;
            avio_close_dyn_buf(_fc->pb, &fileBytes);
            av_freep(&fileBytes);
        }
        else avio_close(_fc->pb);

        _fc->pb = nullptr;
    }

    for(auto& s : _streams)
    {
        if(s.st->codecpar->extradata)
        {
            av_freep(&s.st->codecpar->extradata);
            s.st->codecpar->extradata = nullptr;
            s.st->codecpar->extradata_size = 0;
        }
    }

    if(_fc)
        avformat_free_context(_fc);
}