
#ifndef __r_av_r_options_h
#define __r_av_r_options_h

#include "r_utils/r_nullable.h"
#include "r_utils/r_string_utils.h"

namespace r_av
{

enum r_av_codec_id
{
    r_av_codec_id_h264      = 0,
    r_av_codec_id_hevc      = 1,
    r_av_codec_id_aac       = 2,
    r_av_codec_id_pcm_s16le = 3,
    r_av_codec_id_mp3       = 4,
    r_av_codec_id_mjpeg     = 5
};

enum r_pix_fmt
{
    r_av_pix_fmt_yuv420p = 0,
    r_av_pix_fmt_yuvj420p = 1,
    r_av_pix_fmt_rgba = 2
};

enum r_filter_state
{
    r_filter_state_default,
    r_filter_state_error
};

struct r_stream_options
{
    r_utils::r_nullable<std::string> type;
    r_utils::r_nullable<r_av_codec_id> codec_id;
    r_utils::r_nullable<int> time_base_num;
    r_utils::r_nullable<int> time_base_den;

    // video specfic...
    r_utils::r_nullable<int> width;
    r_utils::r_nullable<int> height;
    r_utils::r_nullable<int> frame_rate_num;
    r_utils::r_nullable<int> frame_rate_den;
    r_utils::r_nullable<r_pix_fmt> format;
    r_utils::r_nullable<std::string> profile;

    // audio specific...
    r_utils::r_nullable<int> bits_per_raw_sample;
    r_utils::r_nullable<int> channels;
    r_utils::r_nullable<int> sample_rate;
};

struct r_codec_options
{
    // video options...
    r_utils::r_nullable<std::string> profile;
    r_utils::r_nullable<std::string> preset;
    r_utils::r_nullable<std::string> tune;
    r_utils::r_nullable<int> thread_count;
};

struct r_codec_options get_decoder_options(int thread_count = 1);

struct r_codec_options get_encoder_options(const std::string& profile, const std::string& preset, const std::string& tune, int thread_count = 1);

}

#endif
