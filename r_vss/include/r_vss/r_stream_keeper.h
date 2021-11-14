
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

namespace r_vss
{

struct r_stream_status
{
    r_disco::r_camera camera;
    uint32_t bytes_per_second;
};

enum r_stream_keeper_commands
{
    R_SK_FETCH_STREAM_STATUS
};

class r_stream_keeper final
{
public:
    r_stream_keeper(r_disco::r_devices& devices, const std::string& top_dir);
    ~r_stream_keeper() noexcept;

    void start();
    void stop();

    std::vector<r_stream_status> fetch_stream_status();

private:
    void _entry_point();
    std::vector<r_disco::r_camera> _get_current_cameras();
    void _add_recording_contexts(const std::vector<r_disco::r_camera>& cameras);
    void _remove_recording_contexts(const std::vector<r_disco::r_camera>& cameras);
    std::vector<r_stream_status> _fetch_stream_status() const;
    r_disco::r_devices& _devices;
    std::string _top_dir;
    std::thread _th;
    bool _running;
    std::map<std::string, std::shared_ptr<r_recording_context>> _streams;
    r_utils::r_work_q<int, std::vector<r_stream_status>> _cmd_q;
};

}

#endif
