
#ifndef r_disco_r_agent_h
#define r_disco_r_agent_h

#include "r_disco/r_stream_config.h"
#include "r_utils/r_timer.h"
#include "r_disco/r_provider.h"
#include <thread>
#include <vector>
#include <memory>
#include <map>

namespace r_disco
{

// - All cameras come from discovery. Manually added cameras still come from a discovery
//   provider (so that all the cameras work the same way), just a special one that is only
//   for manually entered cameras.
//
// - discover onvif cameras
//   for each camera
//     populate stream config structs from RTSP DESCRIBE + onvif info
//     generate stream config hash
//       - camera id is GUID, current IP address is just another stream config attribute
//       - onvif may have a GUID somewhere but if it doesn't, it looks like you can read /proc/net/arp to get HW addresses of recent connections
//

typedef std::function<void(const std::vector<r_stream_config>&)> changed_streams_cb;

class r_agent
{
public:
    r_agent(const std::string& top_dir);
    ~r_agent() noexcept;

    void set_stream_change_cb(changed_streams_cb cb) {_changed_streams_cb = cb;}

    void start();
    void stop();

private:
    void _entry_point();
    std::vector<r_stream_config> _collect_stream_configs();
    void _process_new_or_changed_streams_configs();

    std::thread _th;
    bool _running;
    std::vector<std::shared_ptr<r_provider>> _providers;
    changed_streams_cb _changed_streams_cb;
    std::string _top_dir;
    r_utils::r_timer _timer;
    std::map<std::string, std::string> _device_config_hashes;
};

}

#endif
