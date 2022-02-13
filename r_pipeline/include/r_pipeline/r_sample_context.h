
#ifndef r_pipeline_r_sample_context_h
#define r_pipeline_r_sample_context_h

#include "r_pipeline/r_stream_info.h"
#include "r_utils/r_nullable.h"
#include "r_utils/r_std_utils.h"

#ifdef IS_WINDOWS
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif
#include <gst/gst.h>
#include <gst/rtsp/gstrtspmessage.h>
#include <gst/sdp/gstsdpmessage.h>
#include <gst/app/gstappsink.h>
#ifdef IS_WINDOWS
#pragma warning( pop )
#endif

#include <map>

namespace r_pipeline
{

class r_gst_source;

struct gst_time_info
{
    uint64_t pts {0};
    uint64_t pts_running_time {0};
    uint64_t dts {0};
    uint64_t dts_running_time {0};
};

class sample_context
{
    friend class r_gst_source;

public:
    r_utils::r_nullable<double> framerate() const;
    r_utils::r_nullable<uint8_t> audio_channels() const;
    r_utils::r_nullable<uint32_t> audio_sample_rate() const;
    int64_t stream_start_ts() const;
    std::string sdp() const;
    r_sdp_media sdp_media(r_media type) const;

    uint64_t gst_pts() const;
    uint64_t gst_pts_running_time() const;
    uint64_t gst_dts() const;
    uint64_t gst_dts_running_time() const;

private:
    r_utils::r_nullable<uint8_t> _audio_channels {0};
    r_utils::r_nullable<uint32_t> _audio_sample_rate {0};

    int64_t _stream_start_time {0};

    std::map<r_media, r_pad_info> _src_pad_info {};
    std::string _sdp_text {};
    std::map<std::string, r_sdp_media> _sdp_medias {};

    gst_time_info _gst_time_info {};
};

}

#endif
