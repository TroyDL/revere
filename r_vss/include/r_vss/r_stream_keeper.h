
#ifndef __r_vss_r_stream_keeper_h
#define __r_vss_r_stream_keeper_h

//#include "r_vss/r_recording_context.h"
#include "r_vss/r_motion_engine.h"
#include "r_disco/r_devices.h"
#include "r_disco/r_camera.h"
#include "r_utils/r_nullable.h"
#include "r_utils/r_work_q.h"
#include "r_pipeline/r_stream_info.h"
#include <map>
#include <vector>
#include <thread>
#include <memory>
#include <functional>

struct _GMainLoop;
typedef struct _GMainLoop GMainLoop;
struct _GstRTSPServer;
typedef struct _GstRTSPServer GstRTSPServer;
struct _GstRTSPMountPoints;
typedef struct _GstRTSPMountPoints GstRTSPMountPoints;
struct _GstRTSPMediaFactory;
typedef struct _GstRTSPMediaFactory GstRTSPMediaFactory;
struct _GstRTSPMedia;
typedef _GstRTSPMedia GstRTSPMedia;

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

class r_recording_context;

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

    void post_key_frame_to_motion_engine(r_pipeline::r_gst_buffer buffer, int64_t ts, const std::string& video_codec_name, const std::string& video_codec_params, const std::string& camera_id);

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
    r_motion_engine _motionEngine;
};

}

#endif
