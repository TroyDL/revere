
#ifndef r_discovery_r_discovery_h
#define r_discovery_r_discovery_h

#include "r_utils/r_socket.h"
#include "r_utils/r_socket_address.h"
#include "r_utils/r_server_threaded.h"
#include "r_utils/r_nullable.h"
#include "r_http/r_web_server.h"
#include "r_http/r_server_request.h"
#include "r_http/r_server_response.h"
#include "r_disco/r_recognizer.h"
#include "r_disco/r_device_info.h"

#include <thread>
#include <map>
#include <future>
#include <mutex>
#include <functional>
#include <vector>

namespace r_disco
{

class r_discovery final
{
public:
    r_discovery(const std::string& deviceID,
                const std::string& serverIP = r_utils::ip4_addr_any,
                int serverPort = 80);
    r_discovery(const r_discovery&) = delete;
    ~r_discovery() throw();

    r_discovery& operator = (const r_discovery&) = delete;

    // Use start_discovery to detect devices by periodically sending MSEARCHs and
    // listening for NOTIFYs.
    void start_discovery(const std::string& serviceType, int maxLatency, std::function<void(const r_discovery& disco)> deviceChangedCB);
    std::vector<r_device_info> get_devices() const;

    // Use start_discoverable to respond to listen for MSEARCHs and respond by sending
    // NOTIFYs.
    void start_discoverable(const std::string& locationURL, const std::string& server, const std::string& serviceType);

    void stop();

private:
    void _discovery_entry(const std::string& serviceType, int maxLatency);
    int _cache_drop_old();
    void _update_cache(const r_device_info& di);

    void _discoverable_entry(const std::string& locationURL, const std::string& server, const std::string& serviceType);

    std::thread _worker;
    bool _running;
    int _sok;
    std::string _deviceID;

    mutable std::recursive_mutex _discoveryStateLock;
    std::map<r_device_info,std::chrono::steady_clock::time_point> _deviceCache;
    std::function<void(const r_discovery& disco)> _deviceChangedCB;

    std::string _serverIP;
    int _serverPort;

    r_recognizer _recognizer;
};

}

#endif
