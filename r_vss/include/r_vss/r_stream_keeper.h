
#ifndef __r_vss_r_stream_keeper_h
#define __r_vss_r_stream_keeper_h

#include "r_vss/r_recording_context.h"
#include "r_disco/r_devices.h"
#include "r_disco/r_camera.h"
#include "r_utils/r_nullable.h"
#include "r_utils/r_work_q.h"
#include <map>
#include <vector>
#include <thread>
#include <memory>

struct _GMainLoop;
typedef struct _GMainLoop GMainLoop;
struct _GstRTSPServer;
typedef struct _GstRTSPServer GstRTSPServer;
struct _GstRTSPMountPoints;
typedef struct _GstRTSPMountPoints GstRTSPMountPoints;
struct _GstRTSPMediaFactory;
typedef struct _GstRTSPMediaFactory GstRTSPMediaFactory;

namespace r_vss
{

struct r_stream_status
{
    r_disco::r_camera camera;
    uint32_t bytes_per_second;
};

enum r_stream_keeper_commands
{
    R_SK_FETCH_STREAM_STATUS,
    R_SK_IS_RECORDING
};

struct r_stream_keeper_cmd
{
    r_stream_keeper_commands cmd;
    std::string id;
};

struct r_stream_keeper_result
{
    bool is_recording;
    std::vector<r_stream_status> stream_infos;
};

// How to add RTSP server:
// Example: https://github.com/GStreamer/gst-rtsp-server/blob/master/examples/test-appsrc2.c
//
//   1. add rtsp server stuff to r_stream_keeper.
//   2. add r_stream_keeper back pointers to r_recording_context.
//   3. modify r_recording_context to have callbacks on sdp and ready
//       sdp callback should cache sdp
//       ready callback should add the egress mount and since it has the sdp it can build the right string
//       with the correct payloaders. it will do this with a call to to r_stream_keeper
//   2. factory launch strings should look like this:
//      gst_rtsp_media_factory_set_launch (factory,
//           "( appsrc name=videosrc ! h264parse ! rtph264pay name=pay0 pt=96 "
//           "  appsrc name=audiosrc ! audioconvert ! rtpL24pay name=pay1 pt=97 )");
//      note the appsrc's.
//   3. Connection "media-configure" callback.
//       g_signal_connect (factory, "media-configure", (GCallback) media_configure, recording_context_ptr);
//   4. In media_configure:
//      media will come from sample callbacks of r_gst_source member of r_recording_context
//      get the caps on on the r_gst_source appsink
//      set the caps on the appsrc
//         for both audio and video
//   5. attach need-data callback
//   6. need-data callback should get most recent buffers.
//      need-data is called sometimes with the video appsrc and sometimes with the audio appsrc (whichever needs data)
//      convert the pts's to running times with gst_segment_to_running_time()
//      emit the buffer with g_signal_emit_by_name("push-buffer")
//
//      r_utils needs a new data queue?
//          - readers should be able to block (with a timeout)
//          - writers should not block

class r_stream_keeper final
{
public:
    r_stream_keeper(r_disco::r_devices& devices, const std::string& top_dir);
    ~r_stream_keeper() noexcept;

    void start();
    void stop();

    std::vector<r_stream_status> fetch_stream_status();

    bool is_recording(const std::string& id);

    std::string add_restream_mount(const std::map<std::string, r_pipeline::r_sdp_media>& sdp_medias, const r_disco::r_camera& camera, r_recording_context* rc);
    void remove_restream_mount(const std::string& path);

private:
    void _entry_point();
    void _rtsp_server_entry_point();
    std::vector<r_disco::r_camera> _get_current_cameras();
    void _add_recording_contexts(const std::vector<r_disco::r_camera>& cameras);
    void _remove_recording_contexts(const std::vector<r_disco::r_camera>& cameras);
    std::vector<r_stream_status> _fetch_stream_status() const;
    static void _restream_media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data);
    r_disco::r_devices& _devices;
    std::string _top_dir;
    std::thread _th;
    bool _running;
    std::map<std::string, std::shared_ptr<r_recording_context>> _streams;
    r_utils::r_work_q<r_stream_keeper_cmd, r_stream_keeper_result> _cmd_q;

    std::thread _rtsp_server_th;
    GMainLoop* _loop;
    GstRTSPServer* _server;
    GstRTSPMountPoints* _mounts;
    std::vector<GstRTSPMediaFactory*> _factories;
};

}

#endif
