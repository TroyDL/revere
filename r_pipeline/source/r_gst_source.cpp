
#include "r_pipeline/r_gst_source.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_std_utils.h"
#include "r_utils/r_work_q.h"
#include "r_utils/r_logger.h"
#include <memory>
#include <chrono>

using namespace r_pipeline;
using namespace r_utils;
using namespace r_utils::r_std_utils;
using namespace std;
using namespace std::chrono;

void r_pipeline::gstreamer_init()
{
    gst_init(NULL, NULL);
}

map<string, r_sdp_media> r_pipeline::fetch_sdp_media(
    const string& rtsp_url,
    const r_nullable<string>& username,
    const r_nullable<string>& password
)
{
    vector<r_arg> arguments;
    add_argument(arguments, "url", rtsp_url);

    if(!username.is_null())
        add_argument(arguments, "username", username.value());

    if(!password.is_null())
        add_argument(arguments, "password", password.value());

    r_gst_source src;
    src.set_args(arguments);

    r_work_q<int, void> q;
    map<string, r_sdp_media> medias;
    src.set_sdp_media_cb([&](const map<string, r_sdp_media>& sdp_medias){
        medias = sdp_medias;
    });
    src.set_video_sample_cb([&](const sample_context& sc, const uint8_t*, size_t, bool key, int64_t pts){
        q.post(42);
    });

    src.play();

    auto res = q.poll();

    src.stop();

    return medias;
}

int64_t r_pipeline::fetch_bytes_per_second(
    const std::string& rtsp_url,
    int measured_duration_seconds,
    const r_utils::r_nullable<std::string>& username,
    const r_utils::r_nullable<std::string>& password
)
{
    vector<r_arg> arguments;
    add_argument(arguments, "url", rtsp_url);

    if(!username.is_null())
        add_argument(arguments, "username", username.value());

    if(!password.is_null())
        add_argument(arguments, "password", password.value());

    r_gst_source src;
    src.set_args(arguments);

    bool done = false;

    int64_t audio_byte_total = 0;
    src.set_audio_sample_cb([&](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){
        if(!done)
            audio_byte_total += sz;
    });

    bool stream_start_time_set = false;
    system_clock::time_point stream_start_time;    

    int64_t video_byte_total = 0;
    src.set_video_sample_cb([&](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){
        if(!stream_start_time_set)
        {
            stream_start_time_set = true;
            stream_start_time = system_clock::now();
        }
        if(!done)
            video_byte_total += sz;
    });

    src.play();

    std::this_thread::sleep_for(std::chrono::seconds(measured_duration_seconds));

    done = true;

    auto delta = system_clock::now() - stream_start_time;

    src.stop();

    return (int64_t)(((double)(audio_byte_total + video_byte_total)) / (double)duration_cast<seconds>(delta).count());
}

r_gst_source::r_gst_source() :
    _video_sample_cb(),
    _audio_sample_cb(),
    _ready_cb(),
    _sdp_media_cb(),
    _pad_added_cb(),
    _args(),
    _url(),
    _username(),
    _password(),
    _pipeline(nullptr),
    _bus_watch_id(0),
    _v_appsink(nullptr),
    _a_appsink(nullptr),
    _h264_nal_parser(nullptr),
    _h265_nal_parser(nullptr),
    _sample_context(),
    _video_sample_sent(false),
    _audio_sample_sent(false)
{
}

r_gst_source::~r_gst_source() noexcept
{
    _clear();
}

void r_gst_source::set_args(const vector<r_arg>& args)
{
    _args.clear();
    for(auto arg: args)
        _args[arg.get_name()] = arg.get_value();

    _url = _args["url"];
    if(_url.is_null())
        R_STHROW(r_invalid_argument_exception, ("required argument not found: url"));

    _username = _args["username"];
    _password = _args["password"];
}

void r_gst_source::play()
{
    GstElement* rtspsrc = gst_element_factory_make("rtspsrc", "src");

    g_object_set(G_OBJECT(rtspsrc), "location", _url.value().c_str(), NULL);
    g_object_set(G_OBJECT(rtspsrc), "latency", 50, NULL);

    if(!_username.is_null())
        g_object_set(G_OBJECT(rtspsrc), "user-id", _username.value().c_str(), NULL);
    if(!_password.is_null())
        g_object_set(G_OBJECT(rtspsrc), "user-pw", _password.value().c_str(), NULL);

    g_signal_connect(G_OBJECT(rtspsrc), "pad-added", G_CALLBACK(_pad_added_callbackS), this);

    g_signal_connect(G_OBJECT(rtspsrc), "on-sdp", G_CALLBACK(_on_sdp_callbackS), this);

    _pipeline = gst_pipeline_new("pipe");

    auto added = gst_bin_add(GST_BIN(_pipeline), rtspsrc);

    raii_ptr<GstBus> bus(
        gst_pipeline_get_bus(GST_PIPELINE(_pipeline)),
        [](GstBus* bus){gst_object_unref(bus);}
    );

    _bus_watch_id = gst_bus_add_watch(bus.get(), _bus_callbackS, this);

    gst_element_set_state(_pipeline, GST_STATE_PLAYING);
}

void r_gst_source::stop()
{
    if(_pipeline)
        gst_element_set_state(_pipeline, GST_STATE_NULL);
}

#if 1
static gboolean print_field (GQuark field, const GValue * value, gpointer pfx) {
  gchar *str = gst_value_serialize (value);

  g_print ("%s  %15s: %s\n", (gchar *) pfx, g_quark_to_string (field), str);
  g_free (str);
  return TRUE;
}

static void print_caps (const GstCaps * caps, const gchar * pfx) {
  guint i;

  g_return_if_fail (caps != NULL);

  if (gst_caps_is_any (caps)) {
    g_print ("%sANY\n", pfx);
    return;
  }
  if (gst_caps_is_empty (caps)) {
    g_print ("%sEMPTY\n", pfx);
    return;
  }

  for (i = 0; i < gst_caps_get_size (caps); i++) {
    GstStructure *structure = gst_caps_get_structure (caps, i);

    g_print ("%s%s\n", pfx, gst_structure_get_name (structure));
    gst_structure_foreach (structure, print_field, (gpointer) pfx);
  }
}
#endif

void r_gst_source::_pad_added_callbackS(GstElement* src, GstPad* new_pad, r_gst_source* context)
{
    context->_pad_added_callback(src, new_pad);
}

void r_gst_source::_pad_added_callback(GstElement* src, GstPad* new_pad)
{
    raii_ptr<GstCaps> new_pad_caps(
        gst_pad_get_current_caps(new_pad),
        [](GstCaps* caps){gst_caps_unref(caps);}
    );

    GstStructure* new_pad_struct = gst_caps_get_structure(new_pad_caps.get(), 0);

    const gchar* new_pad_type = gst_structure_get_name(new_pad_struct);

    if(g_str_has_prefix(new_pad_type, "application/x-rtp"))
    {
        r_pad_info si;
        auto media_str = string(gst_structure_get_string(new_pad_struct, "media"));
        si.media = (media_str=="video")?VIDEO_MEDIA:AUDIO_MEDIA;
        si.payload = 0;
        gst_structure_get_int(new_pad_struct, "payload", &si.payload);
        si.clock_rate = 0;
        gst_structure_get_int(new_pad_struct, "clock-rate", &si.clock_rate);

        if(gst_structure_has_field(new_pad_struct, "a-framerate") == TRUE)
            si.framerate.set_value(r_string_utils::s_to_double(gst_structure_get_string(new_pad_struct, "a-framerate")));

        auto encoding = str_to_encoding(string(gst_structure_get_string(new_pad_struct, "encoding-name")));

        if(si.media == VIDEO_MEDIA)
        {
            if(encoding == H264_ENCODING)
            {
                r_h264_info h264_info;
                h264_info.profile_level_id = string(gst_structure_get_string(new_pad_struct, "profile-level-id"));
                h264_info.sprop_parameter_sets = string(gst_structure_get_string(new_pad_struct, "sprop-parameter-sets"));
                h264_info.packetization_mode = 0;
                gst_structure_get_int(new_pad_struct, "packetization-mode", &h264_info.packetization_mode);

                si.encoding = H264_ENCODING;
                si.h264.set_value(h264_info);

                _attach_h264_video_pipeline(new_pad);

                _h264_nal_parser = gst_h264_nal_parser_new();
            }
            else if(encoding == H265_ENCODING)
            {
                r_h265_info h265_info;
                h265_info.sprop_vps = string(gst_structure_get_string(new_pad_struct, "sprop-vps"));
                h265_info.sprop_sps = string(gst_structure_get_string(new_pad_struct, "sprop-sps"));
                h265_info.sprop_pps = string(gst_structure_get_string(new_pad_struct, "sprop-pps"));

                si.encoding = H265_ENCODING;
                si.h265.set_value(h265_info);

                _attach_h265_video_pipeline(new_pad);

                _h265_nal_parser = gst_h265_parser_new();
            }
        }
        else if(si.media == AUDIO_MEDIA)
        {
            if(encoding == AAC_LATM_ENCODING)
            {
                r_aac_info aac_info;

                int value;
                if(gst_structure_get_int(new_pad_struct, "clock-rate", &value) == true)
                    aac_info.clock_rate.set_value(value);

                si.encoding = AAC_LATM_ENCODING;
                si.aac.set_value(aac_info);

                _attach_aac_audio_pipeline(new_pad, encoding);
            }
            else if(encoding == AAC_GENERIC_ENCODING)
            {
                r_aac_info aac_info;

                int value;
                if(gst_structure_get_int(new_pad_struct, "clock-rate", &value) == true)
                    aac_info.clock_rate.set_value(value);

                si.encoding = AAC_GENERIC_ENCODING;
                si.aac.set_value(aac_info);

                _attach_aac_audio_pipeline(new_pad, encoding);
            }
            else if(encoding == PCMU_ENCODING)
            {
                r_pcmu_info pcmu_info;

                int value;
                if(gst_structure_get_int(new_pad_struct, "clock-rate", &value) == true)
                    pcmu_info.clock_rate.set_value(value);

                si.encoding = PCMU_ENCODING;
                si.pcmu.set_value(pcmu_info);

                _attach_g711_audio_pipeline(new_pad);
            }
        }

        _sample_context._src_pad_info[si.media] = si;

        if(!_pad_added_cb.is_null())
            _pad_added_cb.value()(si.media, si);
    }
}

void r_gst_source::_attach_h264_video_pipeline(GstPad* new_pad)
{
    GstElement* v_depay = gst_element_factory_make("rtph264depay", "v_depay");
    GstElement* v_parser = gst_element_factory_make("h264parse", "v_parser");
    _v_appsink = gst_element_factory_make("appsink", "v_appsink");
    g_object_set(G_OBJECT(_v_appsink), "emit-signals", TRUE, "sync", FALSE, NULL);

    gst_bin_add_many(GST_BIN(_pipeline), v_depay, v_parser, _v_appsink, NULL);

    gst_element_sync_state_with_parent(v_depay);
    gst_element_sync_state_with_parent(v_parser);
    gst_element_sync_state_with_parent(_v_appsink);

    raii_ptr<GstCaps> depay_filter_caps(
        gst_caps_new_simple(
            "video/x-h264",
            "stream-format", G_TYPE_STRING, "avc",
            "alignment", G_TYPE_STRING, "au",
            NULL
        ),
        [](GstCaps* caps){gst_caps_unref(caps);}
    );

    auto linked = gst_element_link_filtered(v_depay, v_parser, depay_filter_caps.get());

    raii_ptr<GstCaps> parser_filter_caps(
        gst_caps_new_simple(
            "video/x-h264",
            "stream-format", G_TYPE_STRING, "byte-stream",
            "alignment", G_TYPE_STRING, "au",
            NULL
        ),
        [](GstCaps* caps){gst_caps_unref(caps);}
    );

    auto linked2 = gst_element_link_filtered(v_parser, _v_appsink, parser_filter_caps.get());

    raii_ptr<GstPad> v_depay_sink_pad(
        gst_element_get_static_pad(v_depay, "sink"),
        [](GstPad* pad){gst_object_unref(pad);}
    );

    GstPadLinkReturn ret = gst_pad_link(new_pad, v_depay_sink_pad.get());

    if(GST_PAD_LINK_FAILED(ret))
        R_LOG_ERROR("VIDEO PAD LINK FAILED");

    g_signal_connect(_v_appsink, "new-sample", G_CALLBACK(_new_video_sample), this);
}

void r_gst_source::_attach_h265_video_pipeline(GstPad* new_pad)
{
    GstElement* v_depay = gst_element_factory_make("rtph265depay", "v_depay");
    GstElement* v_parser = gst_element_factory_make("h265parse", "v_parser");
    _v_appsink = gst_element_factory_make("appsink", "v_appsink");
    g_object_set(G_OBJECT(_v_appsink), "emit-signals", TRUE, "sync", FALSE, NULL);

    gst_bin_add_many(GST_BIN(_pipeline), v_depay, v_parser, _v_appsink, NULL);

    gst_element_sync_state_with_parent(v_depay);
    gst_element_sync_state_with_parent(v_parser);
    gst_element_sync_state_with_parent(_v_appsink);

    raii_ptr<GstCaps> depay_filter_caps(
        gst_caps_new_simple(
            "video/x-h265",
            "stream-format", G_TYPE_STRING, "hvc1",
            "alignment", G_TYPE_STRING, "au",
            NULL
        ),
        [](GstCaps* caps){gst_caps_unref(caps);}
    );

    auto linked = gst_element_link_filtered(v_depay, v_parser, depay_filter_caps.get());

    raii_ptr<GstCaps> parser_filter_caps(
        gst_caps_new_simple(
            "video/x-h265",
            "stream-format", G_TYPE_STRING, "byte-stream",
            "alignment", G_TYPE_STRING, "au",
            NULL
        ),
        [](GstCaps* caps){gst_caps_unref(caps);}
    );

    auto linked2 = gst_element_link_filtered(v_parser, _v_appsink, parser_filter_caps.get());

    raii_ptr<GstPad> v_depay_sink_pad(
        gst_element_get_static_pad(v_depay, "sink"),
        [](GstPad* pad){gst_object_unref(pad);}
    );

    GstPadLinkReturn ret = gst_pad_link(new_pad, v_depay_sink_pad.get());

    if(GST_PAD_LINK_FAILED(ret))
        R_LOG_ERROR("VIDEO PAD LINK FAILED");

    g_signal_connect(_v_appsink, "new-sample", G_CALLBACK(_new_video_sample), this);
}

void r_gst_source::_attach_aac_audio_pipeline(GstPad* new_pad, r_encoding encoding)
{
    GstElement* a_depay = (encoding==AAC_LATM_ENCODING)?gst_element_factory_make("rtpmp4adepay", "a_depay"):gst_element_factory_make("rtpmp4gdepay", "a_depay");
    GstElement* a_parser = gst_element_factory_make("aacparse", "a_parser");
    _a_appsink = gst_element_factory_make("appsink", "a_appsink");
    g_object_set(G_OBJECT(_a_appsink), "emit-signals", TRUE, "sync", FALSE, NULL);

    gst_bin_add_many(GST_BIN(_pipeline), a_depay, a_parser, _a_appsink, NULL);

    gst_element_sync_state_with_parent(a_depay);
    gst_element_sync_state_with_parent(a_parser);
    gst_element_sync_state_with_parent(_a_appsink);

    auto linked = gst_element_link_many(a_depay, a_parser, _a_appsink, NULL);

    raii_ptr<GstPad> a_depay_sink_pad(
        gst_element_get_static_pad(a_depay, "sink"),
        [](GstPad* pad){gst_object_unref(pad);}
    );

    GstPadLinkReturn ret = gst_pad_link(new_pad, a_depay_sink_pad.get());

    if(GST_PAD_LINK_FAILED(ret))
        R_LOG_ERROR("AUDIO PAD LINK FAILED");

    g_signal_connect(_a_appsink, "new-sample", G_CALLBACK(_new_audio_sample), this);
}

void r_gst_source::_attach_g711_audio_pipeline(GstPad* new_pad)
{
    GstElement* a_depay = gst_element_factory_make("rtppcmudepay", "a_depay");
    _a_appsink = gst_element_factory_make("appsink", "a_appsink");
    g_object_set(G_OBJECT(_a_appsink), "emit-signals", TRUE, "sync", FALSE, NULL);

    gst_bin_add_many(GST_BIN(_pipeline), a_depay, _a_appsink, NULL);

    gst_element_sync_state_with_parent(a_depay);
    gst_element_sync_state_with_parent(_a_appsink);

    auto linked = gst_element_link_many(a_depay, _a_appsink, NULL);

    raii_ptr<GstPad> a_depay_sink_pad(
        gst_element_get_static_pad(a_depay, "sink"),
        [](GstPad* pad){gst_object_unref(pad);}
    );

    GstPadLinkReturn ret = gst_pad_link(new_pad, a_depay_sink_pad.get());

    if(GST_PAD_LINK_FAILED(ret))
        R_LOG_ERROR("AUDIO PAD LINK FAILED");

    g_signal_connect(_a_appsink, "new-sample", G_CALLBACK(_new_audio_sample), this);
}

GstFlowReturn r_gst_source::_new_video_sample(GstElement* elt, r_gst_source* src)
{
    raii_ptr<GstSample> sample(
        gst_app_sink_pull_sample(GST_APP_SINK(elt)),
        [](GstSample* sample){gst_sample_unref(sample);}
    );

    auto buffer = gst_sample_get_buffer(sample.get());
    if(!buffer)
        return GST_FLOW_ERROR;

    auto result = GST_FLOW_OK;

    GstMapInfo info;
    if(!gst_buffer_map(buffer, &info, GST_MAP_READ))
        result = GST_FLOW_ERROR;

    if(result == GST_FLOW_OK)
    {
        bool key = (src->_h264_nal_parser)?src->_parse_h264(src->_h264_nal_parser, info.data, info.size):src->_parse_h265(src->_h265_nal_parser, info.data, info.size);

        auto sample_pts = GST_BUFFER_PTS(buffer);

        if(sample_pts != GST_CLOCK_TIME_NONE)
        {
            auto pts = (int64_t)GST_TIME_AS_MSECONDS(sample_pts);

            if(!src->_video_sample_sent)
                src->_sample_context._video_stream_start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

            if(!src->_video_sample_cb.is_null())
            {
                src->_video_sample_cb.value()(src->_sample_context, info.data, info.size, key, pts);
                src->_video_sample_sent = true;
            }
        }
    }

    gst_buffer_unmap(buffer, &info);

    return result;
}

GstFlowReturn r_gst_source::_new_audio_sample(GstElement* elt, r_gst_source* src)
{
    raii_ptr<GstSample> sample(
        gst_app_sink_pull_sample(GST_APP_SINK(elt)),
        [](GstSample* sample){gst_sample_unref(sample);}
    );

    auto buffer = gst_sample_get_buffer(sample.get());
    if(!buffer)
        return GST_FLOW_ERROR;

    auto result = GST_FLOW_OK;

    GstMapInfo info;
    if(!gst_buffer_map(buffer, &info, GST_MAP_READ))
        result = GST_FLOW_ERROR;

    if(result == GST_FLOW_OK)
    {
        auto sample_pts = GST_BUFFER_PTS(buffer);

        if(sample_pts != GST_CLOCK_TIME_NONE)
        {
            auto pts = (int64_t)GST_TIME_AS_MSECONDS(sample_pts);

            if(!src->_audio_sample_sent)
            {
                src->_parse_audio_sink_caps();
                src->_sample_context._audio_stream_start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            }

            if(!src->_audio_sample_cb.is_null())
            {
                src->_audio_sample_cb.value()(src->_sample_context, info.data, info.size, true, pts);
                src->_audio_sample_sent = true;
            }
        }
    }

    gst_buffer_unmap(buffer, &info);

    return result;
}

gboolean r_gst_source::_bus_callbackS(GstBus* bus, GstMessage* message, gpointer data)
{
    return ((r_gst_source*)data)->_bus_callback(bus, message);
}

static bool _is_pipeline_msg(GstElement* pipeline, GstMessage* msg)
{
    return GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline);
}

static bool _is_state_change_msg(GstMessage* msg)
{
    return msg->type == GST_MESSAGE_STATE_CHANGED;
}

gboolean r_gst_source::_bus_callback(GstBus* bus, GstMessage* message)
{
    if(_is_pipeline_msg(_pipeline, message) && _is_state_change_msg(message))
    {
        GstState from, to, pending;
        gst_message_parse_state_changed(message, &from, &to, &pending);

        if(to == GST_STATE_READY)
        {
            if(!_ready_cb.is_null())
                _ready_cb.value()();
        }
    }

    return TRUE;
}

void r_gst_source::_on_sdp_callbackS(GstElement* src, GstSDPMessage* sdp, gpointer data)
{
    ((r_gst_source*)data)->_on_sdp_callback(src, sdp);
}

void r_gst_source::_on_sdp_callback(GstElement* src, GstSDPMessage* sdp)
{
    // For details on each field in GstSDPMessage see:
    // https://gstreamer.freedesktop.org/documentation/sdp/gstsdpmessage.html?gi-language=c

    auto sdp_txt = gst_sdp_message_as_text(sdp);

    _sample_context._sdp_text = string(sdp_txt);

    g_free(sdp_txt);

    for(unsigned int i = 0; i < sdp->medias->len; ++i)
    {
        auto m = &g_array_index(sdp->medias, GstSDPMedia, i);

        r_sdp_media media;

        media.type = (string(m->media)=="video")?VIDEO_MEDIA:AUDIO_MEDIA;

        // Note: If its a GArray of object pointers you use & in front of g_array_index

        for(unsigned int ii = 0; ii < m->fmts->len; ++ii)
        {
            // Note: for a string (gchar*) dont put prefix & on g_array_index
            media.formats.push_back(stoi(string(g_array_index(m->fmts, gchar*, ii))));
        }

        for(unsigned int ii = 0; ii < m->attributes->len; ++ii)
        {
            auto attr = &g_array_index(m->attributes, GstSDPAttribute, ii);

            string attr_key(attr->key);
            string attr_val(attr->value);

            // Note: we might want to parse fmtp lines here as well. fmtp attributes occur for each
            // available video stream and are slightly different for h.264
            // fmtp==96 packetization-mode=1;profile-level-id=64000a;sprop-parameter-sets=Z2QACqzZRifmwFqAgICgAAB9IAAXcAHiRLLA,aOvjyyLA
            // and h.265
            // fmtp==96 sprop-vps=QAEMAf//AWAAAAMAkAAAAwAAAwAelZgJ;sprop-sps=QgEBAWAAAAMAkAAAAwAAAwAeoDCDn1llZrkyvAWoCAgIIAAAfSAAC7gB;sprop-pps=RAHBc9GJ

            if(attr_key == "rtpmap")
            {
                // a=rtpmap:96 H264/90000

                auto outer_parts = r_string_utils::split(attr_val, " ");
                auto inner_parts = r_string_utils::split(outer_parts[1], "/");

                r_rtp_map rtpmap;
                rtpmap.encoding = str_to_encoding(inner_parts[0]);
                rtpmap.time_base = stoi(inner_parts[1]);
                media.rtpmaps.insert(make_pair(stoi(outer_parts[0]), rtpmap));
            }
            else if(attr_key == "fmtp")
            {
                auto outer_parts = r_string_utils::split(attr_val, " ");
                auto inner_parts = r_string_utils::split(outer_parts[1], ";");
                for(auto nvp : inner_parts)
                {
                    auto parts = r_string_utils::split(nvp, "=");
                    if(parts.size() == 2)
                        media.attributes.insert(make_pair(r_string_utils::strip(parts[0]), r_string_utils::strip(parts[1])));
                }
            }
            else media.attributes.insert(make_pair(r_string_utils::strip(attr_key), r_string_utils::strip(attr_val)));
        }

        _sample_context._sdp_medias.insert(make_pair(string(m->media), media));
    }

    if(!_sdp_media_cb.is_null())
        _sdp_media_cb.value()(_sample_context._sdp_medias);
}

bool r_gst_source::_parse_h264(GstH264NalParser* parser, const uint8_t* p, size_t size)
{
    bool key = false;
    size_t pos = 0;

    GstH264NalUnit nal_unit;
    while(pos < size)
    {
        gst_h264_parser_identify_nalu(parser, p, (guint)pos, (guint)size, &nal_unit);

        if(nal_unit.type >= GST_H264_NAL_SLICE && nal_unit.type <= GST_H264_NAL_SLICE_IDR)
        {
            GstH264SliceHdr slice_hdr;
            auto pr = gst_h264_parser_parse_slice_hdr(parser, &nal_unit, &slice_hdr, false, false);

            if(pr == GST_H264_PARSER_OK && GST_H264_IS_I_SLICE(&slice_hdr))
                key = true;
        }
        else gst_h264_parser_parse_nal(parser, &nal_unit);

        pos = nal_unit.offset + nal_unit.size;
    }

    return key;
}

bool r_gst_source::_parse_h265(GstH265Parser* parser, const uint8_t* p, size_t size)
{
    bool key = false;
    size_t pos = 0;

    GstH265NalUnit nal_unit;
    while(pos < size)
    {
        gst_h265_parser_identify_nalu(parser, p, (guint)pos, (guint)size, &nal_unit);

        if(nal_unit.type <= GST_H265_NAL_SLICE_CRA_NUT)
        {
            GstH265SliceHdr slice_hdr;
            auto pr = gst_h265_parser_parse_slice_hdr(parser, &nal_unit, &slice_hdr);

            if(pr == GST_H265_PARSER_OK && GST_H265_IS_I_SLICE(&slice_hdr))
                key = true;
        }
        else gst_h265_parser_parse_nal(parser, &nal_unit);

        pos = nal_unit.offset + nal_unit.size;
    }

    return key;
}

void r_gst_source::_parse_audio_sink_caps()
{
    raii_ptr<GstPad> sinkpad(
        gst_element_get_static_pad(_a_appsink, "sink"),
        [](GstPad* pad){gst_object_unref(pad);}
    );

    raii_ptr<GstCaps> caps(
        gst_pad_get_current_caps(sinkpad.get()),
        [](GstCaps* caps){gst_caps_unref(caps);}
    );

    GstStructure* structure = gst_caps_get_structure(caps.get(), 0);
    if(!structure)
        R_THROW(("Unable to query audio appsink sink pad caps."));

    int channels = 0;
    if(gst_structure_get_int(structure, "channels", &channels) == TRUE)
        _sample_context._audio_channels.set_value((uint8_t)channels);

    int rate = 0;
    if(gst_structure_get_int(structure, "rate", &rate) == TRUE)
        _sample_context._audio_sample_rate.set_value((uint32_t)rate);
}

void r_gst_source::_clear() noexcept
{
    if(_h264_nal_parser)
        gst_h264_nal_parser_free(_h264_nal_parser);
    if(!_h265_nal_parser)
        gst_h265_parser_free(_h265_nal_parser);

    if(_pipeline)
    {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        gst_object_unref(_pipeline);
    };
}