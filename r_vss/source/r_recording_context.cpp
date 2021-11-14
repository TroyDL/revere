
#include "r_vss/r_recording_context.h"
#include "r_pipeline/r_arg.h"
#include "r_utils/r_file.h"
#include "r_utils/r_logger.h"
#include <vector>

using namespace r_vss;
using namespace r_disco;
using namespace r_pipeline;
using namespace r_storage;
using namespace r_utils;
using namespace std;
using namespace std::chrono;

r_recording_context::r_recording_context(const r_camera& camera, const string& top_dir) :
    _camera(camera),
    _top_dir(top_dir),
    _source(),
    _storage_file(top_dir + r_fs::PATH_SLASH + "video" + r_fs::PATH_SLASH + camera.record_file_path.value()),
    _storage_write_context(_storage_file.create_write_context(camera.video_codec, camera.video_codec_parameters, camera.audio_codec, camera.audio_codec_parameters)),
    _sample_write_lock(),
    _last_v_time(system_clock::now()),
    _last_a_time(system_clock::now()),
    _has_audio(false),
    _stream_start_ts(system_clock::now()),
    _v_bytes_received(0),
    _a_bytes_received(0)
{
    vector<r_arg> arguments;
    add_argument(arguments, "url", _camera.rtsp_url);

    if(!_camera.rtsp_username.is_null())
        add_argument(arguments, "username", _camera.rtsp_username.value());
    if(!_camera.rtsp_password.is_null())
        add_argument(arguments, "password", _camera.rtsp_password.value());
    
    _source.set_args(arguments);

    _source.set_audio_sample_cb([this](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){
        _has_audio = true;
        _last_a_time = system_clock::now();
        _a_bytes_received += sz;
        lock_guard<mutex> g(this->_sample_write_lock);
        auto ts = sc.audio_stream_start_ts() + pts;
        this->_storage_file.write_frame(
            this->_storage_write_context,
            R_STORAGE_MEDIA_TYPE_AUDIO,
            p,
            sz,
            key,
            ts,
            pts
        );
    });

    _source.set_video_sample_cb([this](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){
        _last_v_time = system_clock::now();
        _v_bytes_received += sz;
        lock_guard<mutex> g(this->_sample_write_lock);
        auto ts = sc.video_stream_start_ts() + pts;
        this->_storage_file.write_frame(
            this->_storage_write_context,
            R_STORAGE_MEDIA_TYPE_VIDEO,
            p,
            sz,
            key,
            ts,
            pts
        );
    });

    R_LOG_INFO("recording: camera.id=%s, file=%s, rtsp_url=%s", _camera.id.c_str(), _camera.record_file_path.value().c_str(), _camera.rtsp_url.c_str());

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
        R_LOG_INFO("found dead stream: camera.id=%s", _camera.id.c_str());

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
