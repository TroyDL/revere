
#include "r_vss/r_stream_keeper.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_std_utils.h"
#include "r_utils/r_functional.h"
#include "r_utils/r_logger.h"
#include "r_utils/r_file.h"
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
    _cmd_q()
{
    auto video_path = top_dir + r_fs::PATH_SLASH + "video";

    if(!r_fs::file_exists(video_path))
        r_fs::mkdir(video_path);
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
                    printf("%s.is_recording=%s\n", cmd.first.id.c_str(), result.is_recording ? "true" : "false");
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
        if(!_streams.count(camera.id))
        {
            printf("stream keeper add\n");
            _streams[camera.id] = make_shared<r_recording_context>(camera, _top_dir);
        }
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
