
#ifndef r_pipeline_r_gst_source_h
#define r_pipeline_r_gst_source_h

#include "r_pipeline/r_arg.h"
#include "r_pipeline/r_stream_info.h"
#include "r_pipeline/r_sample_context.h"
#include "r_utils/r_nullable.h"
#ifdef IS_WINDOWS
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif
#include <gst/gst.h>
#include <gst/rtsp/gstrtspmessage.h>
#include <gst/sdp/gstsdpmessage.h>
#include <gst/app/gstappsink.h>
#define GST_USE_UNSTABLE_API
#include <gst/codecparsers/gsth264parser.h>
#include <gst/codecparsers/gsth265parser.h>
#ifdef IS_WINDOWS
#pragma warning( pop )
#endif
#include <vector>
#include <string>
#include <functional>
#include <map>

namespace r_pipeline
{

void gstreamer_init();

typedef std::function<void()> r_ready_cb;
typedef std::function<void(const sample_context& ctx, const uint8_t* p, size_t sz, bool key, int64_t pts)> r_sample_cb;
typedef std::function<void(const std::map<std::string, r_sdp_media>& sdp_medias)> r_sdp_media_cb;
typedef std::function<void(r_media type, const r_pad_info& pad_info)> r_pad_added_cb;

std::map<std::string, r_sdp_media> fetch_sdp_media(
    const std::string& rtsp_url,
    const r_utils::r_nullable<std::string>& username = r_utils::r_nullable<std::string>(),
    const r_utils::r_nullable<std::string>& password = r_utils::r_nullable<std::string>()
);

int64_t fetch_bytes_per_second(
    const std::string& rtsp_url,
    int measured_duration_seconds = 15,
    const r_utils::r_nullable<std::string>& username = r_utils::r_nullable<std::string>(),
    const r_utils::r_nullable<std::string>& password = r_utils::r_nullable<std::string>()
);

class r_gst_source
{
public:
    r_gst_source();
    r_gst_source(const r_gst_source&) = delete;
    r_gst_source(r_gst_source&&) = delete;
    ~r_gst_source() noexcept;

    r_gst_source& operator=(const r_gst_source&) = delete;
    r_gst_source& operator=(r_gst_source&&) = delete;

    void set_args(const std::vector<r_arg>& args);
    void play();
    void stop();

    void set_video_sample_cb(r_sample_cb cb) {_video_sample_cb = cb;}
    void set_audio_sample_cb(r_sample_cb cb) {_audio_sample_cb = cb;}
    void set_ready_cb(r_ready_cb cb) {_ready_cb = cb;}
    void set_sdp_media_cb(r_sdp_media_cb cb) {_sdp_media_cb = cb;}
    void set_pad_added_cb(r_pad_added_cb cb) {_pad_added_cb = cb;}

private:
    static GstFlowReturn _new_video_sample(GstElement* elt, r_gst_source* src);
    static GstFlowReturn _new_audio_sample(GstElement* elt, r_gst_source* src);

    static void _pad_added_callbackS(GstElement* src, GstPad* new_pad, r_gst_source* context);
    void _pad_added_callback(GstElement* src, GstPad* new_pad);

    void _attach_h264_video_pipeline(GstPad* new_pad);
    void _attach_h265_video_pipeline(GstPad* new_pad);
    void _attach_aac_audio_pipeline(GstPad* new_pad, r_encoding encoding);
    void _attach_g711_audio_pipeline(GstPad* new_pad);

    static gboolean _bus_callbackS(GstBus* bus, GstMessage* message, gpointer data);
    gboolean _bus_callback(GstBus* bus, GstMessage* message);

    static void _on_sdp_callbackS(GstElement* src, GstSDPMessage* sdp, gpointer data);
    void _on_sdp_callback(GstElement* src, GstSDPMessage* sdp);

    bool _parse_h264(GstH264NalParser* parser, const uint8_t* p, size_t size);
    bool _parse_h265(GstH265Parser* parser, const uint8_t* p, size_t size);

    void _parse_audio_sink_caps();

    void _clear() noexcept;

    r_utils::r_nullable<r_sample_cb> _video_sample_cb;
    r_utils::r_nullable<r_sample_cb> _audio_sample_cb;
    r_utils::r_nullable<r_ready_cb> _ready_cb;
    r_utils::r_nullable<r_sdp_media_cb> _sdp_media_cb;
    r_utils::r_nullable<r_pad_added_cb> _pad_added_cb;

    std::map<std::string, r_utils::r_nullable<std::string>> _args;

    /// required arguments
    r_utils::r_nullable<std::string> _url;
    r_utils::r_nullable<std::string> _username;
    r_utils::r_nullable<std::string> _password;

    GstElement* _pipeline;
    guint _bus_watch_id;
    GstElement* _v_appsink;
    GstElement* _a_appsink;

    GstH264NalParser* _h264_nal_parser;
    GstH265Parser* _h265_nal_parser;

    sample_context _sample_context;
    bool _video_sample_sent;
    bool _audio_sample_sent;
};

}

#endif
