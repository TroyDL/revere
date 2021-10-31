
#include "r_vss/r_recording_context.h"
#include "r_pipeline/r_arg.h"
#include <vector>

using namespace r_vss;
using namespace r_disco;
using namespace r_pipeline;
using namespace r_storage;
using namespace std;

r_recording_context::r_recording_context(const r_camera& camera) :
    _camera(camera),
    _source(),
    _storage_file(camera.record_file_path.value()),
    _storage_write_context(_storage_file.create_write_context(camera.video_codec, camera.video_codec_parameters, camera.audio_codec, camera.audio_codec_parameters)),
    _sample_write_lock()
{
    vector<r_arg> arguments;
    add_argument(arguments, "url", _camera.rtsp_url);

    if(!_camera.rtsp_username.is_null())
        add_argument(arguments, "username", _camera.rtsp_username.value());
    if(!_camera.rtsp_password.is_null())
        add_argument(arguments, "password", _camera.rtsp_password.value());
    
    _source.set_args(arguments);

    _source.set_audio_sample_cb([this](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){
        lock_guard<mutex> g(this->_sample_write_lock);
        this->_storage_file.write_frame(
            this->_storage_write_context,
            R_STORAGE_MEDIA_TYPE_AUDIO,
            p,
            sz,
            key,
            sc.audio_stream_start_ts() + pts,
            pts
        );
    });

    _source.set_video_sample_cb([this](const sample_context& sc, const uint8_t* p, size_t sz, bool key, int64_t pts){
        lock_guard<mutex> g(this->_sample_write_lock);
        this->_storage_file.write_frame(
            this->_storage_write_context,
            R_STORAGE_MEDIA_TYPE_VIDEO,
            p,
            sz,
            key,
            sc.video_stream_start_ts() + pts,
            pts
        );
    });

    _source.play();
}

r_recording_context::r_recording_context(r_recording_context&& other) noexcept :
    _camera(move(other._camera)),
    _source(move(other._source)),
    _storage_file(move(other._storage_file)),
    _storage_write_context(move(other._storage_write_context)),
    _sample_write_lock()
{
}

r_recording_context::~r_recording_context() noexcept
{
    _source.stop();
}

r_recording_context& r_recording_context::operator=(r_recording_context&& other) noexcept
{
    _camera = move(other._camera);
    _source = move(other._source);
    _storage_file = move(other._storage_file);
    _storage_write_context = move(other._storage_write_context);
    return *this;
}

r_camera r_recording_context::camera() const
{
    return _camera;
}
