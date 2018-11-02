
#ifndef __r_av_r_utils_h
#define __r_av_r_utils_h

#include "r_av/r_options.h"
#include <string>
extern "C"
{
#include "libavcodec/avcodec.h"
}

namespace r_av
{

AVCodecID r_av_codec_id_to_ffmpeg_codec_id(r_av_codec_id codec_id);
r_av_codec_id ffmpeg_codec_id_to_r_av_codec_id(AVCodecID codec_id);
r_av_codec_id sdp_codec_to_r_av_codec_id(const std::string& codec);

AVPixelFormat r_av_pix_fmt_to_ffmpeg_pix_fmt(r_pix_fmt fmt);
r_pix_fmt ffmpeg_pix_fmt_to_r_av_pix_fmt(AVPixelFormat fmt);

size_t picture_size(r_pix_fmt fmt, uint16_t w, uint16_t h);

std::string filename_to_content_type(const std::string& fileName);

}

#endif
