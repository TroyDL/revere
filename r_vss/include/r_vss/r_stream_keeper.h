
#ifndef __r_vss_r_stream_keeper_h
#define __r_vss_r_stream_keeper_h

#include "r_vss/r_recording_context.h"
#include "r_disco/r_devices.h"
#include <map>
#include <vector>
#include <thread>

namespace r_vss
{

class r_stream_keeper final
{
public:
    r_stream_keeper(r_disco::r_devices& devices);
    ~r_stream_keeper() noexcept;

    void start();
    void stop();

private:
    void _entry_point();
    std::vector<r_disco::r_camera> _get_current_cameras();
    void _add_recording_contexts(const std::vector<r_disco::r_camera>& cameras);
    void _remove_recording_contexts(const std::vector<r_disco::r_camera>& cameras);
    r_disco::r_devices& _devices;
    std::thread _th;
    bool _running;
    std::map<std::string, r_recording_context> _streams;
};

}

#endif
