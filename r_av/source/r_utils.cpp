
#include "r_av/r_utils.h"

using namespace r_av;
using namespace r_utils;
using namespace std;

AVCodecID r_av::r_av_codec_id_to_ffmpeg_codec_id(r_av_codec_id codec_id)
{
    if(codec_id == r_av_codec_id_h264)
        return AV_CODEC_ID_H264;
    else if(codec_id == r_av_codec_id_hevc)
        return AV_CODEC_ID_HEVC;
    else if(codec_id == r_av_codec_id_aac)
        return AV_CODEC_ID_AAC;
    else if(codec_id == r_av_codec_id_pcm_s16le)
        return AV_CODEC_ID_PCM_S16LE;
    else if(codec_id == r_av_codec_id_mp3)
        return AV_CODEC_ID_MP3;
    else if(codec_id == r_av_codec_id_mjpeg)
        return AV_CODEC_ID_MJPEG;

    R_STHROW(r_internal_exception, ("Unknown codec_id"));
}

r_av_codec_id r_av::ffmpeg_codec_id_to_r_av_codec_id(AVCodecID codec_id)
{
    if(codec_id == AV_CODEC_ID_H264)
        return r_av_codec_id_h264;
    else if(codec_id == AV_CODEC_ID_HEVC)
        return r_av_codec_id_hevc;
    else if(codec_id == AV_CODEC_ID_AAC)
        return r_av_codec_id_aac;
    else if(codec_id == AV_CODEC_ID_PCM_S16LE)
        return r_av_codec_id_pcm_s16le;
    else if(codec_id == AV_CODEC_ID_MP3)
        return r_av_codec_id_mp3;
    else if(codec_id == AV_CODEC_ID_MJPEG)
        return r_av_codec_id_mjpeg;

    R_STHROW(r_internal_exception, ("Unknown codec_id"));
}

r_av_codec_id r_av::sdp_codec_to_r_av_codec_id(const std::string& codec)
{
    if(codec == "H264")
        return r_av_codec_id_h264;
    else if(codec == "H265")
        return r_av_codec_id_hevc;

    R_STHROW(r_internal_exception, ("Unknown SDP codec."));
}

AVPixelFormat r_av::r_av_pix_fmt_to_ffmpeg_pix_fmt(r_pix_fmt fmt)
{
    if(fmt == r_av_pix_fmt_yuv420p)
        return AV_PIX_FMT_YUV420P;
    else if(fmt == r_av_pix_fmt_yuvj420p)
        return AV_PIX_FMT_YUVJ420P;
    else if(fmt == r_av_pix_fmt_rgba)
        return AV_PIX_FMT_RGBA;
    
    R_STHROW(r_internal_exception, ("Unknown format"));
}

r_pix_fmt r_av::ffmpeg_pix_fmt_to_r_av_pix_fmt(AVPixelFormat fmt)
{
    if(fmt == AV_PIX_FMT_YUV420P)
        return r_av_pix_fmt_yuv420p;
    else if(fmt == AV_PIX_FMT_YUVJ420P)
        return r_av_pix_fmt_yuvj420p;
    else if(fmt == AV_PIX_FMT_RGBA)
        return r_av_pix_fmt_rgba;
    
    R_STHROW(r_internal_exception, ("Unknown format."));
}

size_t r_av::picture_size(r_pix_fmt fmt, uint16_t w, uint16_t h)
{
    if(fmt == r_av_pix_fmt_rgba)
        return (size_t)((w * 4) * h);
    else if(fmt == r_av_pix_fmt_yuv420p || fmt == r_av_pix_fmt_yuvj420p)
        return (size_t)(w * h * 1.5);

    R_STHROW(r_internal_exception, ("Unknown format."));
}

string r_av::filename_to_content_type(const string& fileName)
{
    if(r_string::contains(fileName, ".mp4"))
        return "video/mp4";
    else if(r_string::contains(fileName, ".ts"))
        return "video/ts";
    else if(r_string::contains(fileName, ".mkv"))
        return "video/mkv";
    else if(r_string::contains(fileName, ".flv"))
        return "video/flv";
    else if(r_string::contains(fileName, ".avi"))
        return "video/avi";
    else if(r_string::contains(fileName, ".mov"))
        return "video/mov";

    R_THROW(("Unknown container: %s",fileName.c_str()));
}
