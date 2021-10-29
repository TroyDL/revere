
#include "r_vss/r_recording_context.h"
#include "r_pipeline/r_arg.h"
#include <vector>

using namespace r_vss;
using namespace r_disco;
using namespace r_pipeline;
using namespace std;

r_recording_context::r_recording_context(const r_camera& camera) :
    _camera(camera),
    _source()
{
    vector<r_arg> arguments;
    add_argument(arguments, "url", _camera.rtsp_url);

    if(!_camera.rtsp_username.is_null())
        add_argument(arguments, "username", _camera.rtsp_username.value());
    if(!_camera.rtsp_password.is_null())
        add_argument(arguments, "password", _camera.rtsp_password.value());
    
    _source.set_args(arguments);

    // set _source video & audio sample cb's

    _source.play();

}

r_recording_context::~r_recording_context() noexcept
{
    _source.stop();
}

r_camera r_recording_context::camera() const
{
    return _camera;
}
