
#include "r_pipeline/r_sample_context.h"

using namespace r_pipeline;
using namespace r_utils;
using namespace std;

r_nullable<double> sample_context::framerate() const
{
    return _src_pad_info.at(VIDEO_MEDIA).framerate;
}

r_nullable<uint8_t> sample_context::audio_channels() const
{
    return _audio_channels;
}

r_nullable<uint32_t> sample_context::audio_sample_rate() const
{
    return _audio_sample_rate;
}

int64_t sample_context::video_stream_start_ts() const
{
    return _video_stream_start_time;
}

int64_t sample_context::audio_stream_start_ts() const
{
    return _audio_stream_start_time;
}

string sample_context::sdp() const
{
    return _sdp_text;
}

r_sdp_media sample_context::sdp_media(r_media type) const
{
    return _sdp_medias.at((type==VIDEO_MEDIA)?"video":"audio");
}
