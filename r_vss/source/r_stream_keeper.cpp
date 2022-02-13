

#include "r_vss/r_stream_keeper.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_std_utils.h"
#include "r_utils/r_functional.h"
#include "r_utils/r_logger.h"
#include "r_utils/r_file.h"

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

#include <algorithm>

using namespace r_vss;
using namespace r_utils;
using namespace r_utils::r_std_utils;
using namespace r_disco;
using namespace std;

r_stream_keeper::r_stream_keeper(r_devices& devices, const string& top_dir) :
    _devices(devices),
    _top_dir(top_dir),
    _th(),
    _running(false),
    _streams(),
    _cmd_q(),
    _rtsp_server_th(),
    _loop(g_main_loop_new(NULL, FALSE)),
    _server(gst_rtsp_onvif_server_new()),
    _mounts(nullptr),
    _factories()
{
    auto video_path = top_dir + r_fs::PATH_SLASH + "video";

    if(!r_fs::file_exists(video_path))
        r_fs::mkdir(video_path);

    g_object_set(_server, "service", to_string(10554).c_str(), NULL);

    _mounts = gst_rtsp_server_get_mount_points(_server);

    gst_rtsp_server_attach(_server, NULL);
}

r_stream_keeper::~r_stream_keeper() noexcept
{
    if(_running)
        stop();

    for(auto f : _factories)
        g_object_unref(f);

    if(_mounts)
        g_object_unref(_mounts);

    if(_server)
        g_object_unref(_server);

    if(_loop)
        g_main_loop_unref(_loop);
}

void r_stream_keeper::start()
{
    if(_running)
        R_THROW(("Stream keeper already started!"));

    _running = true;

    _rtsp_server_th = thread(&r_stream_keeper::_rtsp_server_entry_point, this);

    _th = thread(&r_stream_keeper::_entry_point, this);
}

void r_stream_keeper::stop()
{
    if(!_running)
        R_THROW(("Cannot stop stream keeper if its not running!"));

    _running = false;

    g_main_loop_quit(_loop);

    _th.join();

    _rtsp_server_th.join();
}

vector<r_stream_status> r_stream_keeper::fetch_stream_status()
{
    r_stream_keeper_cmd cmd;
    cmd.cmd = R_SK_FETCH_STREAM_STATUS;

    return _cmd_q.post(cmd).get().stream_infos;
}

bool r_stream_keeper::is_recording(const string& id)
{
    r_stream_keeper_cmd cmd;
    cmd.cmd = R_SK_IS_RECORDING;
    cmd.id = id;

    return _cmd_q.post(cmd).get().is_recording;
}

std::string r_stream_keeper::add_restream_mount(const std::map<std::string, r_pipeline::r_sdp_media>& sdp_medias, const r_disco::r_camera& camera, r_recording_context* rc)
{
    auto factory = gst_rtsp_media_factory_new();
    if(!factory)
        R_THROW(("Failed to create restream media factory!"));

    // fetch video encoding (it's mandatory)
    if(sdp_medias.find("video") == sdp_medias.end())
        R_THROW(("No video stream found in SDP!"));

    auto& sdp_video = sdp_medias.at("video");

    if(sdp_video.formats.empty())
        R_THROW(("No video formats found in SDP!"));

    auto& sdp_video_formats = sdp_video.formats;

    auto video_encoding = sdp_video.rtpmaps.at(sdp_video_formats.front()).encoding;

    // fetch audio encoding (it's not mandatory)
    r_nullable<r_pipeline::r_encoding> audio_encoding;
    r_nullable<r_pipeline::r_sdp_media> sdp_audio;

    if(sdp_medias.find("audio") != sdp_medias.end())
    {
        sdp_audio.set_value(sdp_medias.at("audio"));

        if(sdp_audio.value().formats.empty())
            R_THROW(("No audio formats found in SDP!"));

        auto& sdp_audio_formats = sdp_audio.value().formats;

        audio_encoding.set_value(sdp_audio.value().rtpmaps.at(sdp_audio_formats.front()).encoding);
    }

    string launch_str = r_string_utils::format("( appsrc name=videosrc ! ");

    if(video_encoding == r_pipeline::r_encoding::H264_ENCODING)
        launch_str += r_string_utils::format("h264parse ! rtph264pay name=pay0 pt=%d ", sdp_video.formats.front());
    else if(video_encoding == r_pipeline::r_encoding::H265_ENCODING)
        launch_str += r_string_utils::format("h265parse ! rtph265pay name=pay0 pt=%d ", sdp_video.formats.front());
    else
        R_THROW(("Unsupported video encoding: %s", r_pipeline::encoding_to_str(video_encoding).c_str()));

    if(!audio_encoding.is_null())
    {
        launch_str += string(" appsrc name=audiosrc ! ");

        if(audio_encoding.value() == r_pipeline::r_encoding::AAC_GENERIC_ENCODING)
            launch_str += r_string_utils::format("aacparse ! rtpmp4gpay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::AAC_LATM_ENCODING)
            launch_str += r_string_utils::format("aacparse ! rtpmp4apay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::PCMU_ENCODING)
            launch_str += r_string_utils::format("rawaudioparse ! rtppcmupay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::PCMA_ENCODING)
            launch_str += r_string_utils::format("rawaudioparse ! rtppcmapay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::AAL2_G726_16_ENCODING)
            launch_str += r_string_utils::format("rtpg726pay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::AAL2_G726_24_ENCODING)
            launch_str += r_string_utils::format("rtpg726pay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::AAL2_G726_32_ENCODING)
            launch_str += r_string_utils::format("rtpg726pay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::AAL2_G726_40_ENCODING)
            launch_str += r_string_utils::format("rtpg726pay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::G726_16_ENCODING)
            launch_str += r_string_utils::format("rtpg726pay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::G726_24_ENCODING)
            launch_str += r_string_utils::format("rtpg726pay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::G726_32_ENCODING)
            launch_str += r_string_utils::format("rtpg726pay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else if(audio_encoding.value() == r_pipeline::r_encoding::G726_40_ENCODING)
            launch_str += r_string_utils::format("rtpg726pay name=pay1 pt=%d )", sdp_audio.value().formats.front());
        else R_THROW(("Unsupported audio encoding: %s", r_pipeline::encoding_to_str(audio_encoding.value()).c_str()));
    }
    else launch_str += ")";

    gst_rtsp_media_factory_set_launch(factory, launch_str.c_str());

    g_signal_connect(factory, "media-configure", (GCallback)_restream_media_configure, rc);

    string mount_path;
    if(!camera.friendly_name.is_null())
        mount_path = camera.friendly_name.value();
    else if(!camera.ipv4.is_null())
        mount_path = camera.ipv4.value();
    else R_THROW(("No friendly name or ipv4 address found for camera!"));

    replace(begin(mount_path), end(mount_path), ' ', '_');

    mount_path = r_string_utils::format("/%s", mount_path.c_str());

    gst_rtsp_mount_points_add_factory(_mounts, mount_path.c_str(), factory);

    return mount_path;
}

void r_stream_keeper::remove_restream_mount(const string& mount_path)
{
    gst_rtsp_mount_points_remove_factory(_mounts, mount_path.c_str());
}

void r_stream_keeper::_entry_point()
{
    while(_running)
    {
        try
        {
            if(_streams.empty())
                _add_recording_contexts(_devices.get_assigned_cameras());
            else
            {
                auto cameras = _get_current_cameras();

                _remove_recording_contexts(_devices.get_modified_cameras(cameras));
                _remove_recording_contexts(_devices.get_assigned_cameras_removed(cameras));
                _add_recording_contexts(_devices.get_assigned_cameras_added(_get_current_cameras()));
            }

            // remove dead recording contexts...
            r_funky::erase_if(_streams, [](const auto& c){return c.second->dead();});

            auto c = _cmd_q.poll(std::chrono::seconds(2));

            if(!c.is_null())
            {
                auto cmd = move(c.take());

                if(cmd.first.cmd == R_SK_FETCH_STREAM_STATUS)
                {
                    r_stream_keeper_result result;
                    result.stream_infos = _fetch_stream_status();
                    cmd.second.set_value(result);
                }
                else if(cmd.first.cmd == R_SK_IS_RECORDING)
                {
                    r_stream_keeper_result result;
                    result.is_recording = _streams.find(cmd.first.id) != _streams.end();
                    cmd.second.set_value(result);
                }
                else R_THROW(("Unknown command sent to stream keeper!"));
            }
        }
        catch(const std::exception& e)
        {
            R_LOG_ERROR("Recording Exception: %s", e.what());
        }
    }
}

void r_stream_keeper::_rtsp_server_entry_point()
{
    g_main_loop_run(_loop);
}

vector<r_camera> r_stream_keeper::_get_current_cameras()
{
    vector<r_camera> cameras;
    cameras.reserve(_streams.size());
    for(auto& c : _streams)
        cameras.push_back(c.second->camera());
    return cameras;
}

void r_stream_keeper::_add_recording_contexts(const vector<r_camera>& cameras)
{
    for(const auto& camera : cameras)
    {
        if(_streams.count(camera.id) == 0)
        {
            printf("stream keeper add camera.id=%s\n", camera.id.c_str());
            _streams[camera.id] = make_shared<r_recording_context>(this, camera, _top_dir);
        }
    }
}

void r_stream_keeper::_remove_recording_contexts(const std::vector<r_disco::r_camera>& cameras)
{
    for(const auto& camera : cameras)
    {
        if(_streams.count(camera.id) > 0)
            _streams.erase(camera.id);
    }
}

vector<r_stream_status> r_stream_keeper::_fetch_stream_status() const
{
    vector<r_stream_status> statuses;
    statuses.reserve(_streams.size());

    transform(
        _streams.begin(),
        _streams.end(),
        back_inserter(statuses),
        [](const auto& c){
            r_stream_status s;
            s.camera = c.second->camera();
            s.bytes_per_second = c.second->bytes_per_second();
            return s;
        }
    );

    return statuses;
}

void r_stream_keeper::_restream_media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data)
{
    ((r_recording_context*)user_data)->restream_media_configure(factory, media);
}