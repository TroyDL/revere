
#ifdef IS_WINDOWS
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif
#ifdef IS_LINUX
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#ifdef IS_LINUX
#pragma GCC diagnostic pop
#endif
#ifdef IS_WINDOWS
#pragma warning( pop )
#endif

#include "r_vss/r_recording_context.h"
#include "r_vss/r_stream_keeper.h"
#include "r_pipeline/r_arg.h"
#include "r_utils/r_file.h"
#include "r_utils/r_logger.h"
#include "r_utils/r_time_utils.h"
#include <vector>

using namespace r_vss;
using namespace r_disco;
using namespace r_pipeline;
using namespace r_storage;
using namespace r_utils;
using namespace std;
using namespace std::chrono;

r_recording_context::r_recording_context(r_stream_keeper* sk, const r_camera& camera, const string& top_dir) :
    _sk(sk),
    _camera(camera),
    _top_dir(top_dir),
    _source(),
    _storage_file(top_dir + r_fs::PATH_SLASH + "video" + r_fs::PATH_SLASH + camera.record_file_path.value()),
    _storage_write_context(_storage_file.create_write_context(camera.video_codec.value(), camera.video_codec_parameters.value(), camera.audio_codec.value(), camera.audio_codec_parameters.value())),
    _sample_write_lock(),
    _last_v_time(system_clock::now()),
    _last_a_time(system_clock::now()),
    _has_audio(false),
    _stream_start_ts(system_clock::now()),
    _v_bytes_received(0),
    _a_bytes_received(0),
    _sdp_medias(),
    _video_caps(),
    _audio_caps(),
    _v_appsrc(nullptr),
    _a_appsrc(nullptr),
    _video_samples(),
    _audio_samples()
{
    vector<r_arg> arguments;
    add_argument(arguments, "url", _camera.rtsp_url.value());

    if(!_camera.rtsp_username.is_null())
        add_argument(arguments, "username", _camera.rtsp_username.value());
    if(!_camera.rtsp_password.is_null())
        add_argument(arguments, "password", _camera.rtsp_password.value());
    
    _source.set_args(arguments);

    _source.set_audio_sample_cb([this](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){
        int64_t ts = 0;
        
        {
            _has_audio = true;
            _last_a_time = system_clock::now();
            _a_bytes_received += sz;
            lock_guard<mutex> g(this->_sample_write_lock);
            if(this->_audio_caps.is_null())
            {
                this->_audio_caps = _source.get_audio_caps();
                if(!this->_video_caps.is_null())
                    this->_sk->add_restream_mount(_sdp_medias, _camera, this);
            }

            ts = sc.stream_start_ts() + pts;
            this->_storage_file.write_frame(
                this->_storage_write_context,
                R_STORAGE_MEDIA_TYPE_AUDIO,
                p,
                sz,
                key,
                ts,
                pts
            );
        }

        {
            _frame_context fc;
            fc.gst_pts = sc.gst_pts_running_time();
            fc.gst_dts = sc.gst_dts_running_time();
            fc.key = key;
            fc.data.resize(sz);
            memcpy(fc.data.data(), p, sz);
            _audio_samples.post(fc);
        }
    });

    _source.set_video_sample_cb([this](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){

        int64_t ts = 0;

        {
            _last_v_time = system_clock::now();
            _v_bytes_received += sz;
            lock_guard<mutex> g(this->_sample_write_lock);
            if(this->_video_caps.is_null())
            {
                this->_video_caps = _source.get_video_caps();
                if(!this->_has_audio || !this->_audio_caps.is_null())
                    this->_sk->add_restream_mount(_sdp_medias, _camera, this);
            }

            ts = sc.stream_start_ts() + pts;
            this->_storage_file.write_frame(
                this->_storage_write_context,
                R_STORAGE_MEDIA_TYPE_VIDEO,
                p,
                sz,
                key,
                ts,
                pts
            );
        }

        {
            _frame_context fc;
            fc.gst_pts = sc.gst_pts_running_time();
            fc.gst_dts = sc.gst_dts_running_time();
            fc.key = key;
            fc.data.resize(sz);
            memcpy(fc.data.data(), p, sz);
            _video_samples.post(fc);
        }
    });

    _source.set_sdp_media_cb([this](const map<string, r_sdp_media>& sdp_medias){
        this->_sdp_medias = sdp_medias;
        if(this->_sdp_medias.find("audio") != this->_sdp_medias.end())
            this->_has_audio = true;
    });

    R_LOG_INFO("recording: camera.id=%s, file=%s, rtsp_url=%s", _camera.id.c_str(), _camera.record_file_path.value().c_str(), _camera.rtsp_url.value().c_str());

    _source.play();
}

r_recording_context::~r_recording_context() noexcept
{
    _source.stop();
}

bool r_recording_context::dead() const
{
    auto now = system_clock::now();
    auto video_dead = ((now - _last_v_time) > seconds(20));
    bool is_dead = (_has_audio)?((now - _last_a_time) > seconds(20))||video_dead:video_dead;

    if(is_dead)
    {
        R_LOG_INFO("found dead stream: camera.id=%s", _camera.id.c_str());
        printf("found dead stream: camera.id=%s\n", _camera.id.c_str());

        printf("SECONDS SINCE LAST VIDEO: %ld\n",duration_cast<seconds>(now - _last_v_time).count());
        printf("SECONDS SINCE LAST AUDIO: %ld\n",duration_cast<seconds>(now - _last_a_time).count());
        fflush(stdout);
    }

    return is_dead;
}

r_camera r_recording_context::camera() const
{
    return _camera;
}

int32_t r_recording_context::bytes_per_second() const
{
    return (_v_bytes_received + _a_bytes_received) / duration_cast<seconds>(system_clock::now() - _stream_start_ts).count();
}

void r_recording_context::restream_media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media)
{
    auto element = gst_rtsp_media_get_element(media);
    if(!element)
        R_THROW(("Failed to get element from media in restream media configure."));


    _v_appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "videosrc");
    if(!_v_appsrc)
        R_THROW(("Failed to get videosrc from element in restream media configure."));

    gst_util_set_object_arg(G_OBJECT(_v_appsrc), "format", "time");

    g_object_set(G_OBJECT(_v_appsrc), "caps", (GstCaps*)_video_caps.value(), NULL);

    g_signal_connect(_v_appsrc, "need-data", (GCallback)r_recording_context::_need_data, this);


    _a_appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "audiosrc");

    if(_a_appsrc)
    {
        gst_util_set_object_arg(G_OBJECT(_a_appsrc), "format", "time");

        g_object_set(G_OBJECT(_a_appsrc), "caps", (GstCaps*)_audio_caps.value(), NULL);

        g_signal_connect(_a_appsrc, "need-data", (GCallback)r_recording_context::_need_data, this);
    }




    // 1) create a video caps
    //     - need a mime type like "video/x-h264"
    //     - stream_format will always be "byte-stream"
    //     - alignment will always be "au"
    //     - need width and height
    //     - need framerate
    //  caps = gst_caps_new_simple ("video/x-h264",
    //           "stream-format", G_TYPE_STRING, "byte-stream",
    //           "alignment", G_TYPE_STRING, "au",
    //           "width", G_TYPE_INT, 384, "height", G_TYPE_INT, 288,
    //           "framerate", GST_TYPE_FRACTION, 15, 1, NULL);
    //
    // 2) on the appsrc set "format" to "time"
    //     gst_util_set_object_arg (G_OBJECT (appsrc), "format", "time");
    // 3) set the caps on the appsrc
    //     g_object_set (G_OBJECT (appsrc), "caps", caps, NULL);
    // 4) install the callback on appsrc that is called when a buffer is needed
    //     g_signal_connect (appsrc, "need-data", (GCallback) need_data, ctx);
    // 5) unref the caps
    //     gst_caps_unref (caps);
    // 6) create audio caps (maybe)
    //     - need mime type
    //         "audio/aac"          AAC
    //         "audio/basic"        MULAW
    //         "audio/x-alaw-basic" ALAW
    //         "audio/x-g726"       G726

}

void r_recording_context::_need_data(GstElement* appsrc, guint unused, r_recording_context* rc)
{
    rc->need_data(appsrc, unused);
}

void r_recording_context::need_data(GstElement* appsrc, guint unused)
{
//        if(_last_video_sample.size() < sz)
//            _last_video_sample.resize(sz);
//        memcpy(_last_video_sample.data(), p, sz);
//        _last_video_ts = ts;

    GstBuffer* buffer = nullptr;

    if(appsrc == _v_appsrc)
    {
        auto sample = _video_samples.poll();
        if(!sample.is_null())
        {
            printf("v [%lu} ", sample.value().data.size());
            buffer = gst_buffer_new_and_alloc(sample.value().data.size());
            GST_BUFFER_PTS(buffer) = sample.value().gst_pts;
            GST_BUFFER_DTS(buffer) = sample.value().gst_dts;
            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_WRITE);
            memcpy(map.data, sample.value().data.data(), sample.value().data.size());
            gst_buffer_unmap(buffer, &map);
        }
    }
    else
    {
        auto sample = _audio_samples.poll();
        if(!sample.is_null())
        {
            printf("a [%lu} ", sample.value().data.size());
            buffer = gst_buffer_new_and_alloc(sample.value().data.size());
            GST_BUFFER_PTS(buffer) = sample.value().gst_pts;
            GST_BUFFER_DTS(buffer) = sample.value().gst_dts;
            GstMapInfo map;
            gst_buffer_map(buffer, &map, GST_MAP_WRITE);
            memcpy(map.data, sample.value().data.data(), sample.value().data.size());
            gst_buffer_unmap(buffer, &map);
        }
    }

    if(buffer)
    {
        int ret;
        g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

        printf("ret=%d\n",ret);

        gst_buffer_unref(buffer);
    }
}
