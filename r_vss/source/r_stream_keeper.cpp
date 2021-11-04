
#include "r_vss/r_stream_keeper.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_std_utils.h"
#include "r_utils/r_functional.h"
#include <algorithm>

using namespace r_vss;
using namespace r_utils;
using namespace r_utils::r_std_utils;
using namespace r_disco;
using namespace std;

r_stream_keeper::r_stream_keeper(r_devices& devices) :
    _devices(devices),
    _th(),
    _running(false)
{
}

r_stream_keeper::~r_stream_keeper() noexcept
{
    if(_running)
        stop();
}

void r_stream_keeper::start()
{
    if(_running)
        R_THROW(("Stream keeper already started!"));

    _running = true;
    _th = thread(&r_stream_keeper::_entry_point, this);
}

void r_stream_keeper::stop()
{
    if(!_running)
        R_THROW(("Cannot stop stream keeper if its not running!"));

    _running = false;
    _th.join();
}

void r_stream_keeper::_entry_point()
{
    while(_running)
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
        r_funky::erase_if(_streams, [](const auto& c){return c.second.dead();});

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

vector<r_camera> r_stream_keeper::_get_current_cameras()
{
    vector<r_camera> cameras;
    cameras.reserve(_streams.size());
    for(auto& c : _streams)
        cameras.push_back(c.second.camera());
    return cameras;
}

void r_stream_keeper::_add_recording_contexts(const vector<r_camera>& cameras)
{
    for(const auto& camera : cameras)
    {
        if(!_streams.count(camera.id))
            _streams.emplace(camera.id, camera);
    }
}

void r_stream_keeper::_remove_recording_contexts(const std::vector<r_disco::r_camera>& cameras)
{
    for(const auto& camera : cameras)
    {
        if(_streams.count(camera.id))
            _streams.erase(camera.id);
    }
}
